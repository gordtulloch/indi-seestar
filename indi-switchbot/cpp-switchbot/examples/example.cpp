#include <iostream>
#include <thread>
#include <chrono>
#include <switchbot/ble_controller.hpp>

int main() {
    try {
        std::cout << "Initializing Bluetooth..." << std::endl;
        switchbot::ble::BLEController controller;
        
        std::cout << "\n=== Scanning for SwitchBot devices (10 seconds) ===" << std::endl;
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
            std::cout << "  RSSI: " << device.rssi << " dBm" << std::endl;
        }
        
        // Test controlling the first device
        if (!devices.empty()) {
            const auto& device = devices[0];
            std::cout << "\n=== Testing control of first device ===" << std::endl;
            std::cout << "Device: " << device.name << " (" << device.address << ")" << std::endl;
            
            std::cout << "\nSending PRESS command..." << std::endl;
            if (controller.bot_command(device.address, "press")) {
                std::cout << "✓ Press command sent successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to send press command" << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            std::cout << "\nSending ON command..." << std::endl;
            if (controller.bot_command(device.address, "on")) {
                std::cout << "✓ ON command sent successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to send ON command" << std::endl;
            }
            
            std::this_thread::sleep_for(std::chrono::seconds(2));
            
            std::cout << "\nSending OFF command..." << std::endl;
            if (controller.bot_command(device.address, "off")) {
                std::cout << "✓ OFF command sent successfully" << std::endl;
            } else {
                std::cout << "✗ Failed to send OFF command" << std::endl;
            }
            
            std::cout << "\n" << std::string(60, '=') << std::endl;
            std::cout << "To control this device from command line:" << std::endl;
            std::cout << "  sudo ./test_bot_ble " << device.address << " press" << std::endl;
            std::cout << "  sudo ./test_bot_ble " << device.address << " on" << std::endl;
            std::cout << "  sudo ./test_bot_ble " << device.address << " off" << std::endl;
            std::cout << std::string(60, '=') << std::endl;
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}
