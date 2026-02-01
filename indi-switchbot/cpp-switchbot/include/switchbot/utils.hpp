#pragma once

#include <string>
#include <nlohmann/json.hpp>

namespace switchbot {

/**
 * Utility functions for the SwitchBot library
 */
namespace utils {

/**
 * Convert string to lowercase
 */
std::string to_lower(const std::string& str);

/**
 * Convert string from snake_case to camelCase
 */
std::string to_camel_case(const std::string& snake_case);

/**
 * Convert string from camelCase to snake_case
 */
std::string to_snake_case(const std::string& camel_case);

/**
 * Convert JSON object keys from camelCase to snake_case recursively
 */
nlohmann::json decamelize_json(const nlohmann::json& obj);

/**
 * Convert JSON object keys from snake_case to camelCase recursively
 */
nlohmann::json camelize_json(const nlohmann::json& obj);

/**
 * Generate HMAC-SHA256 signature in base64
 */
std::string hmac_sha256_base64(const std::string& key, const std::string& data);

/**
 * Get current timestamp in milliseconds
 */
long long get_timestamp_ms();

} // namespace utils

} // namespace switchbot
