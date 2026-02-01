#include "switchbot/ble_controller.hpp"
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/l2cap.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <cstring>
#include <stdexcept>
#include <iostream>
#include <fstream>
#include <thread>
#include <chrono>

namespace switchbot {
namespace ble {

// SwitchBot BLE Service UUID: cba20d00-224d-11e6-9fb8-0002a5d5c51b
// SwitchBot characteristic for write: cba20002-224d-11e6-9fb8-0002a5d5c51b
// SwitchBot characteristic for read: cba20003-224d-11e6-9fb8-0002a5d5c51b

class BLEController::Impl {
public:
    int dev_id;
    int sock;

    Impl() : dev_id(-1), sock(-1) {
        dev_id = hci_get_route(nullptr);
        if (dev_id < 0) {
            throw std::runtime_error("No Bluetooth adapter found");
        }

        sock = hci_open_dev(dev_id);
        if (sock < 0) {
            throw std::runtime_error("Failed to open Bluetooth device");
        }
    }

    ~Impl() {
        if (sock >= 0) {
            close(sock);
        }
    }
};

BLEController::BLEController() : pImpl(std::make_unique<Impl>()) {
}

BLEController::~BLEController() = default;

std::vector<BLEDevice> BLEController::detect_bots(int timeout_seconds) {
    std::vector<BLEDevice> devices;
    
    std::cout << "Detecting SwitchBot devices (scanning for " << timeout_seconds << " seconds)..." << std::endl;
    
    // Use hcitool to scan for BLE devices
    std::string scan_cmd = "timeout " + std::to_string(timeout_seconds) + 
                          " hcitool -i hci0 lescan --duplicates 2>&1 | grep -i 'switchbot\\|wohand' > ./ble_scan.txt &";
    system(scan_cmd.c_str());
    
    // Wait for scan to complete
    std::this_thread::sleep_for(std::chrono::seconds(timeout_seconds + 1));
    
    // Parse results
    std::ifstream scan_file("./ble_scan.txt");
    std::string line;
    while (std::getline(scan_file, line)) {
        // Parse lines like: "E1:3D:05:06:25:90 WoHand"
        size_t space_pos = line.find(' ');
        if (space_pos != std::string::npos) {
            BLEDevice device;
            device.address = line.substr(0, space_pos);
            device.name = line.substr(space_pos + 1);
            device.device_type = "Bot";
            device.rssi = 0;
            
            // Avoid duplicates
            bool found = false;
            for (const auto& d : devices) {
                if (d.address == device.address) {
                    found = true;
                    break;
                }
            }
            if (!found) {
                devices.push_back(device);
                std::cout << "Found: " << device.name << " (" << device.address << ")" << std::endl;
            }
        }
    }
    
    if (devices.empty()) {
        std::cout << "No SwitchBot devices found." << std::endl;
        std::cout << "Make sure devices are powered on and in range." << std::endl;
    }
    
    return devices;
}

std::vector<BLEDevice> BLEController::scan(int timeout_seconds) {
    // Use detect_bots for now
    return detect_bots(timeout_seconds);
}

bool BLEController::bot_command(const std::string& address, const std::string& command) {
    // SwitchBot Bot BLE Protocol:
    // Service UUID: cba20d00-224d-11e6-9fb8-0002a5d5c51b
    // Write characteristic: cba20002-224d-11e6-9fb8-0002a5d5c51b
    //
    // Commands:
    // Turn On:  0x570101
    // Turn Off: 0x570102
    // Press:    0x570100
    
    std::cout << "Sending BLE command '" << command << "' to " << address << std::endl;
    
    // For now, using gatttool as a workaround
    // A full implementation would use BlueZ D-Bus API or direct GATT
    
    std::string cmd_hex;
    if (command == "on") {
        cmd_hex = "570101";
    } else if (command == "off") {
        cmd_hex = "570102";
    } else if (command == "press") {
        cmd_hex = "570100";
    } else {
        throw std::invalid_argument("Invalid command: " + command);
    }
    
    // Use gatttool to send command
    std::string gatttool_cmd = "timeout 10 gatttool -b " + address + 
                               " -t random --char-write-req -a 0x0013 -n " + cmd_hex +
                               " 2>/dev/null";
    
    int result = system(gatttool_cmd.c_str());
    
    return result == 0;
}

std::string BLEController::bot_status(const std::string& address) {
    // Read status from device
    // This is a simplified implementation
    return "unknown";
}

} // namespace ble
} // namespace switchbot
