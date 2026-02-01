#pragma once

#include <string>
#include <vector>
#include <memory>

namespace switchbot {
namespace ble {

/**
 * BLE Device information
 */
struct BLEDevice {
    std::string address;
    std::string name;
    std::string device_type;
    int rssi;
};

/**
 * SwitchBot BLE Controller
 * Communicates directly with SwitchBot devices via Bluetooth
 */
class BLEController {
public:
    BLEController();
    ~BLEController();

    /**
     * Detect and return all SwitchBot Bot devices
     * @param timeout_seconds Scan duration in seconds
     * @return Vector of Bot devices found
     */
    std::vector<BLEDevice> detect_bots(int timeout_seconds = 5);

    /**
     * Scan for SwitchBot devices
     * @param timeout_seconds Scan duration in seconds
     * @return Vector of discovered devices
     */
    std::vector<BLEDevice> scan(int timeout_seconds = 5);

    /**
     * Send command to Bot device
     * @param address Bluetooth MAC address
     * @param command Command: "on", "off", "press"
     * @return true if successful
     */
    bool bot_command(const std::string& address, const std::string& command);

    /**
     * Get Bot device status
     * @param address Bluetooth MAC address
     * @return Status string: "on" or "off"
     */
    std::string bot_status(const std::string& address);

private:
    class Impl;
    std::unique_ptr<Impl> pImpl;
};

} // namespace ble
} // namespace switchbot
