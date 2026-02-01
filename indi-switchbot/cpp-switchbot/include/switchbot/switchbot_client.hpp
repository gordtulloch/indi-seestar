#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>

namespace switchbot {

/**
 * HTTP client for SwitchBot API
 */
class SwitchBotClient {
public:
    /**
     * Constructor
     * @param token SwitchBot API token
     * @param secret SwitchBot API secret
     * @param nonce Nonce for authentication
     */
    SwitchBotClient(const std::string& token, const std::string& secret, const std::string& nonce);

    /**
     * Destructor
     */
    ~SwitchBotClient();

    /**
     * Make a generic HTTP request
     * @param method HTTP method (GET, POST, PUT, DELETE)
     * @param path API path
     * @param payload Optional JSON payload for POST/PUT requests
     * @return Response JSON
     */
    nlohmann::json request(const std::string& method, const std::string& path, 
                          const nlohmann::json& payload = nlohmann::json());

    /**
     * Make a GET request
     * @param path API path
     * @return Response JSON
     */
    nlohmann::json get(const std::string& path);

    /**
     * Make a POST request
     * @param path API path
     * @param payload JSON payload
     * @return Response JSON
     */
    nlohmann::json post(const std::string& path, const nlohmann::json& payload);

    /**
     * Make a PUT request
     * @param path API path
     * @param payload JSON payload
     * @return Response JSON
     */
    nlohmann::json put(const std::string& path, const nlohmann::json& payload);

    /**
     * Make a DELETE request
     * @param path API path
     * @return Response JSON
     */
    nlohmann::json delete_request(const std::string& path);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace switchbot
