#include <iostream>
#include <cstdlib>
#include <switchbot/ble_controller.hpp>

int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <bluetooth_address> <on|off|press>" << std::endl;
        std::cerr << "Example: " << argv[0] << " E1:3D:05:06:25:90 on" << std::endl;
        std::cerr << "\nTo find your device address, use: sudo hcitool lescan" << std::endl;
        return 1;
    }
    
    std::string address = argv[1];
    std::string command = argv[2];
    
    if (command != "on" && command != "off" && command != "press") {
        std::cerr << "Error: Command must be 'on', 'off', or 'press'" << std::endl;
        return 1;
    }
    
    try {
        std::cout << "Initializing Bluetooth..." << std::endl;
        switchbot::ble::BLEController controller;
        
        std::cout << "Sending command to device..." << std::endl;
        bool success = controller.bot_command(address, command);
        
        if (success) {
            std::cout << "Success! Bot turned " << command << std::endl;
            return 0;
        } else {
            std::cerr << "Failed to send command" << std::endl;
            return 1;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
}
