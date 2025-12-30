#pragma once

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <optional>
#include <string>

#include <nlohmann/json.hpp>

namespace seestar
{

struct EquatorialCoords
{
    // Seestar returns RA in hours, Dec in degrees.
    double raHours {0.0};
    double decDegrees {0.0};
};

class SeestarClient
{
public:
    SeestarClient();
    ~SeestarClient();

    SeestarClient(const SeestarClient&) = delete;
    SeestarClient& operator=(const SeestarClient&) = delete;

    void setEndpoint(std::string host, uint16_t port);

    bool connect();
    void disconnect();
    bool isConnected() const;

    // Low-level request/response.
    nlohmann::json call(std::string method, nlohmann::json params = nullptr, std::chrono::milliseconds timeout = std::chrono::seconds(10));

    // High-level helpers.
    std::optional<EquatorialCoords> getEquatorialCoords();
    bool gotoRaDec(double raHours, double decDegrees);
    bool syncRaDec(double raHours, double decDegrees);
    bool abortGoto();

private:
    bool ensureConnectedLocked();
    bool writeLineLocked(const std::string& line);
    std::optional<std::string> readLineLocked(std::chrono::milliseconds timeout);

    std::string host_ {"127.0.0.1"};
    uint16_t port_ {5555};

    mutable std::mutex mutex_;

    int sock_ {-1};
    std::atomic<bool> connected_ {false};
    uint32_t nextId_ {10000};
};

} // namespace seestar
