#include <iostream>
#include <switchbot/ble_controller.hpp>

int main() {
    try {
        std::cout << "Initializing Bluetooth..." << std::endl;
        switchbot::ble::BLEController controller;
        
        auto devices = controller.detect_bots(10);
        
        if (devices.empty()) {
            std::cout << "\nNo SwitchBot Bot devices found." << std::endl;
            std::cout << "Make sure your device is:" << std::endl;
            std::cout << "  - Powered on" << std::endl;
            std::cout << "  - Within Bluetooth range" << std::endl;
            std::cout << "  - Not connected to another app" << std::endl;
            return 1;
        }
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "Found " << devices.size() << " SwitchBot device(s):" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        
        for (size_t i = 0; i < devices.size(); ++i) {
            const auto& device = devices[i];
            std::cout << "\nDevice " << (i + 1) << ":" << std::endl;
            std::cout << "  Name: " << device.name << std::endl;
            std::cout << "  Address: " << device.address << std::endl;
            std::cout << "  Type: " << device.device_type << std::endl;
            
            std::cout << "\n  To control this device:" << std::endl;
            std::cout << "    sudo ./build/test_bot_ble " << device.address << " on" << std::endl;
            std::cout << "    sudo ./build/test_bot_ble " << device.address << " off" << std::endl;
            std::cout << "    sudo ./build/test_bot_ble " << device.address << " press" << std::endl;
        }
        
        std::cout << "\n" << std::string(60, '=') << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
