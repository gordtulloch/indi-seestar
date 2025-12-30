#include "seestar_client.h"

#include <cerrno>
#include <cstring>
#include <memory>

#ifdef __linux__
#include <arpa/inet.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#endif

namespace seestar
{

namespace
{
#ifdef __linux__
struct FdCloser
{
    int fd {-1};
    ~FdCloser()
    {
        if (fd >= 0)
            ::close(fd);
    }
};
#endif
}

SeestarClient::SeestarClient() = default;

SeestarClient::~SeestarClient()
{
    disconnect();
}

void SeestarClient::setEndpoint(std::string host, uint16_t port)
{
    std::lock_guard<std::mutex> lock(mutex_);
    host_ = std::move(host);
    port_ = port;
}

bool SeestarClient::connect()
{
    std::lock_guard<std::mutex> lock(mutex_);

#ifdef __linux__
    if (connected_ && sock_ >= 0)
        return true;

    addrinfo hints {};
    hints.ai_family = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;

    addrinfo* result = nullptr;
    const std::string portStr = std::to_string(port_);
    if (::getaddrinfo(host_.c_str(), portStr.c_str(), &hints, &result) != 0)
        return false;

    std::unique_ptr<addrinfo, decltype(&freeaddrinfo)> resGuard(result, &freeaddrinfo);

    int fd = -1;
    for (addrinfo* rp = result; rp != nullptr; rp = rp->ai_next)
    {
        fd = ::socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
        if (fd < 0)
            continue;

        if (::connect(fd, rp->ai_addr, rp->ai_addrlen) == 0)
            break;

        ::close(fd);
        fd = -1;
    }

    if (fd < 0)
        return false;

    sock_ = fd;
    connected_ = true;
    return true;
#else
    // This project targets Linux deployment under indiserver.
    return false;
#endif
}

void SeestarClient::disconnect()
{
    std::lock_guard<std::mutex> lock(mutex_);

#ifdef __linux__
    connected_ = false;
    if (sock_ >= 0)
    {
        ::close(sock_);
        sock_ = -1;
    }
#else
    connected_ = false;
    sock_ = -1;
#endif
}

bool SeestarClient::isConnected() const
{
    return connected_;
}

bool SeestarClient::ensureConnectedLocked()
{
    if (connected_ && sock_ >= 0)
        return true;

    return connect();
}

bool SeestarClient::writeLineLocked(const std::string& line)
{
#ifdef __linux__
    if (sock_ < 0)
        return false;

    const char* buf = line.c_str();
    size_t remaining = line.size();

    while (remaining > 0)
    {
        const ssize_t sent = ::send(sock_, buf, remaining, 0);
        if (sent < 0)
        {
            if (errno == EINTR)
                continue;
            connected_ = false;
            return false;
        }
        buf += sent;
        remaining -= static_cast<size_t>(sent);
    }

    return true;
#else
    (void)line;
    return false;
#endif
}

std::optional<std::string> SeestarClient::readLineLocked(std::chrono::milliseconds timeout)
{
#ifdef __linux__
    if (sock_ < 0)
        return std::nullopt;

    timeval tv {};
    tv.tv_sec = static_cast<long>(timeout.count() / 1000);
    tv.tv_usec = static_cast<long>((timeout.count() % 1000) * 1000);
    ::setsockopt(sock_, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));

    std::string out;
    out.reserve(512);

    char ch = 0;
    while (true)
    {
        const ssize_t n = ::recv(sock_, &ch, 1, 0);
        if (n == 0)
        {
            connected_ = false;
            return std::nullopt;
        }
        if (n < 0)
        {
            if (errno == EINTR)
                continue;
            // timeout or other socket error
            return std::nullopt;
        }

        out.push_back(ch);

        const size_t len = out.size();
        if (len >= 2 && out[len - 2] == '\r' && out[len - 1] == '\n')
        {
            out.resize(len - 2);
            return out;
        }

        if (out.size() > 1024 * 1024)
        {
            // Safety: avoid unbounded growth.
            return std::nullopt;
        }
    }
#else
    (void)timeout;
    return std::nullopt;
#endif
}

nlohmann::json SeestarClient::call(std::string method, nlohmann::json params, std::chrono::milliseconds timeout)
{
    std::lock_guard<std::mutex> lock(mutex_);

    if (!ensureConnectedLocked())
        return { {"error", "not connected"} };

    const uint32_t id = nextId_++;

    nlohmann::json req;
    req["id"] = id;
    req["method"] = std::move(method);
    if (!params.is_null())
        req["params"] = std::move(params);

    const std::string payload = req.dump() + "\r\n";
    if (!writeLineLocked(payload))
        return { {"error", "send failed"} };

    const auto start = std::chrono::steady_clock::now();
    while (std::chrono::steady_clock::now() - start < timeout)
    {
        const auto remaining = std::chrono::duration_cast<std::chrono::milliseconds>(timeout - (std::chrono::steady_clock::now() - start));
        const auto line = readLineLocked(remaining);
        if (!line.has_value())
            continue;

        nlohmann::json resp;
        try
        {
            resp = nlohmann::json::parse(*line);
        }
        catch (...)
        {
            continue;
        }

        // Seestar sends unsolicited event messages too; only return matching id.
        if (resp.contains("id") && resp["id"].is_number_unsigned() && resp["id"].get<uint32_t>() == id)
            return resp;
    }

    return { {"error", "timeout"}, {"id", id} };
}

std::optional<EquatorialCoords> SeestarClient::getEquatorialCoords()
{
    const auto resp = call("scope_get_equ_coord", nullptr, std::chrono::seconds(5));
    if (!resp.contains("result"))
        return std::nullopt;

    const auto& result = resp["result"];
    if (!result.contains("ra") || !result.contains("dec"))
        return std::nullopt;

    EquatorialCoords coords;
    coords.raHours = result["ra"].get<double>();
    coords.decDegrees = result["dec"].get<double>();
    return coords;
}

bool SeestarClient::gotoRaDec(double raHours, double decDegrees)
{
    auto resp = call("scope_goto", nlohmann::json::array({raHours, decDegrees}), std::chrono::seconds(5));
    return !resp.contains("error");
}

bool SeestarClient::syncRaDec(double raHours, double decDegrees)
{
    auto resp = call("scope_sync", nlohmann::json::array({raHours, decDegrees}), std::chrono::seconds(5));
    return !resp.contains("error");
}

bool SeestarClient::abortGoto()
{
    nlohmann::json params;
    params["stage"] = "AutoGoto";
    auto resp = call("iscope_stop_view", params, std::chrono::seconds(5));
    return !resp.contains("error");
}

} // namespace seestar
