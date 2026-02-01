#include <iostream>
#include <cstdlib>
#include <switchbot/switchbot.hpp>

int main(int argc, char* argv[]) {
    // Check command line arguments
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <on|off>" << std::endl;
        return 1;
    }
    
    std::string command = argv[1];
    if (command != "on" && command != "off") {
        std::cerr << "Error: Command must be 'on' or 'off'" << std::endl;
        return 1;
    }
    
    // Load credentials from environment variables
    const char* token_env = std::getenv("TOKEN");
    const char* secret_env = std::getenv("SECRET");
    
    if (!token_env || !secret_env) {
        std::cerr << "Error: TOKEN and SECRET environment variables must be set" << std::endl;
        std::cerr << "Example: export TOKEN=your_token" << std::endl;
        std::cerr << "         export SECRET=your_secret" << std::endl;
        return 1;
    }
    
    std::string token = token_env;
    std::string secret = secret_env;
    
    try {
        // Create SwitchBot instance
        std::cout << "Connecting to SwitchBot API..." << std::endl;
        switchbot::SwitchBot sb(token, secret);
        
        // Get all devices and find the first Bot
        std::cout << "Fetching devices..." << std::endl;
        auto devices = sb.devices();
        
        switchbot::Bot* bot = nullptr;
        std::string bot_id;
        
        for (auto& device : devices) {
            if (device->get_type() == "Bot") {
                bot = dynamic_cast<switchbot::Bot*>(device.get());
                bot_id = device->get_id();
                break;
            }
        }
        
        if (!bot) {
            std::cerr << "Error: No Bot device found" << std::endl;
            std::cerr << "Available devices:" << std::endl;
            for (const auto& device : devices) {
                std::cerr << "  - " << device->to_string() << std::endl;
            }
            return 1;
        }
        
        // Get current status
        std::cout << "Found Bot: " << bot_id << std::endl;
        auto status = bot->status();
        std::string current_state = status.value("power", "unknown");
        std::cout << "Current state: " << current_state << std::endl;
        
        // Execute command
        std::cout << "Turning " << command << "..." << std::endl;
        bot->turn(command);
        
        std::cout << "Success! Bot turned " << command << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
