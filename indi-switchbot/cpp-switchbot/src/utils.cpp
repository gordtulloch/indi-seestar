#include "switchbot/utils.hpp"
#include <algorithm>
#include <cctype>
#include <sstream>
#include <chrono>
#include <openssl/hmac.h>
#include <openssl/bio.h>
#include <openssl/buffer.h>
#include <openssl/evp.h>

namespace switchbot {
namespace utils {

std::string to_lower(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
}

std::string to_camel_case(const std::string& snake_case) {
    std::string result;
    bool capitalize_next = false;
    
    for (char c : snake_case) {
        if (c == '_') {
            capitalize_next = true;
        } else if (capitalize_next) {
            result += std::toupper(c);
            capitalize_next = false;
        } else {
            result += c;
        }
    }
    
    return result;
}

std::string to_snake_case(const std::string& camel_case) {
    std::string result;
    
    for (size_t i = 0; i < camel_case.length(); ++i) {
        char c = camel_case[i];
        if (std::isupper(c)) {
            if (i > 0) {
                result += '_';
            }
            result += std::tolower(c);
        } else {
            result += c;
        }
    }
    
    return result;
}

nlohmann::json decamelize_json(const nlohmann::json& obj) {
    if (!obj.is_object()) {
        return obj;
    }
    
    nlohmann::json result;
    for (auto& [key, value] : obj.items()) {
        std::string new_key = to_snake_case(key);
        if (value.is_object()) {
            result[new_key] = decamelize_json(value);
        } else if (value.is_array()) {
            nlohmann::json array_result = nlohmann::json::array();
            for (const auto& item : value) {
                if (item.is_object()) {
                    array_result.push_back(decamelize_json(item));
                } else {
                    array_result.push_back(item);
                }
            }
            result[new_key] = array_result;
        } else {
            result[new_key] = value;
        }
    }
    
    return result;
}

nlohmann::json camelize_json(const nlohmann::json& obj) {
    if (!obj.is_object()) {
        return obj;
    }
    
    nlohmann::json result;
    for (auto& [key, value] : obj.items()) {
        std::string new_key = to_camel_case(key);
        if (value.is_object()) {
            result[new_key] = camelize_json(value);
        } else if (value.is_array()) {
            nlohmann::json array_result = nlohmann::json::array();
            for (const auto& item : value) {
                if (item.is_object()) {
                    array_result.push_back(camelize_json(item));
                } else {
                    array_result.push_back(item);
                }
            }
            result[new_key] = array_result;
        } else {
            result[new_key] = value;
        }
    }
    
    return result;
}

std::string hmac_sha256_base64(const std::string& key, const std::string& data) {
    unsigned char hash[EVP_MAX_MD_SIZE];
    unsigned int hash_len;
    
    HMAC(EVP_sha256(), key.c_str(), key.length(),
         reinterpret_cast<const unsigned char*>(data.c_str()), data.length(),
         hash, &hash_len);
    
    // Base64 encode
    BIO* bio = BIO_new(BIO_s_mem());
    BIO* b64 = BIO_new(BIO_f_base64());
    BIO_set_flags(b64, BIO_FLAGS_BASE64_NO_NL);
    bio = BIO_push(b64, bio);
    
    BIO_write(bio, hash, hash_len);
    BIO_flush(bio);
    
    BUF_MEM* buffer_ptr;
    BIO_get_mem_ptr(bio, &buffer_ptr);
    
    std::string result(buffer_ptr->data, buffer_ptr->length);
    BIO_free_all(bio);
    
    return result;
}

long long get_timestamp_ms() {
    auto now = std::chrono::system_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();
}

} // namespace utils
} // namespace switchbot
