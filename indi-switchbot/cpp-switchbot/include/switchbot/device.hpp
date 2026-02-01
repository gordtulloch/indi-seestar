#pragma once

#include <string>
#include <memory>
#include <map>
#include <vector>
#include <nlohmann/json.hpp>
#include "switchbot/switchbot_client.hpp"

namespace switchbot {

/**
 * Base class for all SwitchBot devices
 */
class Device {
public:
    /**
     * Constructor
     * @param client Shared pointer to SwitchBotClient
     * @param id Device ID
     * @param extra Additional device properties from API
     */
    Device(std::shared_ptr<SwitchBotClient> client, const std::string& id, 
           const nlohmann::json& extra = nlohmann::json());

    virtual ~Device() = default;

    /**
     * Factory method to create appropriate device type
     * @param client Shared pointer to SwitchBotClient
     * @param id Device ID
     * @param extra Additional device properties from API
     * @return Unique pointer to Device
     */
    static std::unique_ptr<Device> create(std::shared_ptr<SwitchBotClient> client,
                                         const std::string& id,
                                         const nlohmann::json& extra);

    /**
     * Get device status
     * @return JSON object with device status
     */
    nlohmann::json status();

    /**
     * Send command to device
     * @param action Command action name
     * @param parameter Optional command parameter
     */
    void command(const std::string& action, const std::string& parameter = "default");

    /**
     * Get string representation of device
     */
    virtual std::string to_string() const;

    // Getters
    std::string get_id() const { return id; }
    std::string get_name() const { return name; }
    std::string get_type() const { return type; }
    bool is_cloud_enabled() const { return cloud_enabled; }
    std::string get_hub_id() const { return hub_id; }

protected:
    std::shared_ptr<SwitchBotClient> client;
    std::string id;
    std::string name;
    std::string type;
    bool cloud_enabled;
    std::string hub_id;
};

/**
 * Bot device (simple switch/button)
 */
class Bot : public Device {
public:
    using Device::Device;

    /**
     * Turn bot on or off
     * @param state "on" or "off"
     */
    void turn(const std::string& state);

    /**
     * Press the bot button
     */
    void press();

    /**
     * Toggle bot state
     */
    void toggle();
};

/**
 * Curtain device
 */
class Curtain : public Device {
public:
    Curtain(std::shared_ptr<SwitchBotClient> client, const std::string& id,
            const nlohmann::json& extra);

    std::vector<std::string> get_curtain_ids() const { return curtain_ids; }
    bool is_calibrated() const { return calibrated; }
    bool is_grouped() const { return grouped; }
    bool is_master() const { return master; }
    std::string get_open_direction() const { return open_direction; }

private:
    std::vector<std::string> curtain_ids;
    bool calibrated;
    bool grouped;
    bool master;
    std::string open_direction;
};

/**
 * Smart Lock device
 */
class Lock : public Device {
public:
    using Device::Device;

    /**
     * Lock the device
     */
    void lock();

    /**
     * Unlock the device
     */
    void unlock();

    /**
     * Toggle lock state
     */
    void toggle();
};

} // namespace switchbot
