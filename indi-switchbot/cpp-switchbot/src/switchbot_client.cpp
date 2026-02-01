#include "switchbot/switchbot_client.hpp"
#include "switchbot/utils.hpp"
#include <curl/curl.h>
#include <stdexcept>
#include <sstream>

namespace switchbot {

const std::string SWITCHBOT_HOST = "https://api.switch-bot.com/v1.1";

// Callback for CURL to write response data
static size_t write_callback(void* contents, size_t size, size_t nmemb, std::string* userp) {
    userp->append(static_cast<char*>(contents), size * nmemb);
    return size * nmemb;
}

class SwitchBotClient::Impl {
public:
    std::string token;
    std::string secret;
    std::string nonce;
    
    Impl(const std::string& token, const std::string& secret, const std::string& nonce)
        : token(token), secret(secret), nonce(nonce) {
    }
};

SwitchBotClient::SwitchBotClient(const std::string& token, const std::string& secret, 
                                 const std::string& nonce)
    : pImpl(std::make_unique<Impl>(token, secret, nonce)) {
    curl_global_init(CURL_GLOBAL_DEFAULT);
}

SwitchBotClient::~SwitchBotClient() {
    curl_global_cleanup();
}

nlohmann::json SwitchBotClient::request(const std::string& method, const std::string& path,
                                       const nlohmann::json& payload) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        throw std::runtime_error("Failed to initialize CURL");
    }
    
    std::string url = SWITCHBOT_HOST + "/" + path;
    std::string response_string;
    struct curl_slist* headers = nullptr;
    
    // Generate fresh headers for this request
    long long timestamp = utils::get_timestamp_ms();
    std::string signature_data = pImpl->token + std::to_string(timestamp) + pImpl->nonce;
    std::string signature = utils::hmac_sha256_base64(pImpl->secret, signature_data);
    
    std::string authorization_header = "Authorization: " + pImpl->token;
    std::string timestamp_header = "t: " + std::to_string(timestamp);
    std::string sign_header = "sign: " + signature;
    std::string nonce_header = "nonce: " + pImpl->nonce;
    
    // Debug output
    // std::cerr << "URL: " << url << std::endl;
    // std::cerr << "Method: " << method << std::endl;
    // std::cerr << "Timestamp: " << timestamp << std::endl;
    // std::cerr << "Signature data: " << signature_data << std::endl;
    // std::cerr << "Signature: " << signature << std::endl;
    
    headers = curl_slist_append(headers, authorization_header.c_str());
    headers = curl_slist_append(headers, timestamp_header.c_str());
    headers = curl_slist_append(headers, sign_header.c_str());
    headers = curl_slist_append(headers, nonce_header.c_str());
    headers = curl_slist_append(headers, "Content-Type: application/json");
    
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response_string);
    
    // Prepare payload string (must remain in scope for the entire request)
    std::string payload_str;
    if (!payload.empty()) {
        payload_str = payload.dump();
    }
    
    // Set method and payload
    if (method == "POST") {
        curl_easy_setopt(curl, CURLOPT_POST, 1L);
        if (!payload_str.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload_str.length());
        }
    } else if (method == "PUT") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "PUT");
        if (!payload_str.empty()) {
            curl_easy_setopt(curl, CURLOPT_POSTFIELDS, payload_str.c_str());
            curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, payload_str.length());
        }
    } else if (method == "DELETE") {
        curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, "DELETE");
    }
    
    CURLcode res = curl_easy_perform(curl);
    
    long http_code = 0;
    curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &http_code);
    
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        throw std::runtime_error("CURL request failed: " + std::string(curl_easy_strerror(res)));
    }
    
    if (http_code != 200) {
        std::string error_msg = "SwitchBot API server returns status " + std::to_string(http_code);
        if (!response_string.empty()) {
            error_msg += "\nResponse: " + response_string;
        }
        throw std::runtime_error(error_msg);
    }
    
    nlohmann::json response_json = nlohmann::json::parse(response_string);
    nlohmann::json response_decamelized = utils::decamelize_json(response_json);
    
    int status_code = response_decamelized.value("status_code", 0);
    if (status_code != 100) {
        std::string message = response_decamelized.value("message", "Unknown error");
        throw std::runtime_error("An error occurred: " + message);
    }
    
    return response_decamelized;
}

nlohmann::json SwitchBotClient::get(const std::string& path) {
    return request("GET", path);
}

nlohmann::json SwitchBotClient::post(const std::string& path, const nlohmann::json& payload) {
    return request("POST", path, payload);
}

nlohmann::json SwitchBotClient::put(const std::string& path, const nlohmann::json& payload) {
    return request("PUT", path, payload);
}

nlohmann::json SwitchBotClient::delete_request(const std::string& path) {
    return request("DELETE", path);
}

} // namespace switchbot
