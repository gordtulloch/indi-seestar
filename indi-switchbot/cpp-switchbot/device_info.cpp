#include <iostream>
#include <iomanip>
#include <switchbot/switchbot.hpp>

int main() {
    const char* token_env = std::getenv("TOKEN");
    const char* secret_env = std::getenv("SECRET");
    
    if (!token_env || !secret_env) {
        std::cerr << "Error: TOKEN and SECRET environment variables must be set" << std::endl;
        return 1;
    }
    
    try {
        switchbot::SwitchBot sb(token_env, secret_env);
        
        std::cout << "=== Fetching all devices ===" << std::endl;
        auto devices = sb.devices();
        
        for (const auto& device : devices) {
            std::cout << "\n" << std::string(50, '=') << std::endl;
            std::cout << "Device: " << device->to_string() << std::endl;
            std::cout << "  ID: " << device->get_id() << std::endl;
            std::cout << "  Name: " << device->get_name() << std::endl;
            std::cout << "  Type: " << device->get_type() << std::endl;
            std::cout << "  Cloud Enabled: " << (device->is_cloud_enabled() ? "YES" : "NO") << std::endl;
            std::cout << "  Hub ID: " << device->get_hub_id() << std::endl;
            
            // Try to get status
            try {
                std::cout << "  Status: " << std::endl;
                auto status = device->status();
                std::cout << status.dump(4) << std::endl;
            } catch (const std::exception& e) {
                std::cout << "  Status Error: " << e.what() << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
