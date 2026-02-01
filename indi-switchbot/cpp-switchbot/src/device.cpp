#include "switchbot/device.hpp"
#include "switchbot/utils.hpp"
#include <stdexcept>

namespace switchbot {

Device::Device(std::shared_ptr<SwitchBotClient> client, const std::string& id,
               const nlohmann::json& extra)
    : client(client), id(id), cloud_enabled(false) {
    
    if (extra.contains("device_name")) {
        name = extra["device_name"].get<std::string>();
    }
    if (extra.contains("device_type")) {
        type = extra["device_type"].get<std::string>();
    }
    if (extra.contains("enable_cloud_service")) {
        cloud_enabled = extra["enable_cloud_service"].get<bool>();
    }
    if (extra.contains("hub_device_id")) {
        hub_id = extra["hub_device_id"].get<std::string>();
    }
}

std::unique_ptr<Device> Device::create(std::shared_ptr<SwitchBotClient> client,
                                      const std::string& id,
                                      const nlohmann::json& extra) {
    std::string device_type;
    if (extra.contains("device_type")) {
        device_type = extra["device_type"].get<std::string>();
    }
    
    if (device_type == "Bot") {
        return std::make_unique<Bot>(client, id, extra);
    } else if (device_type == "Curtain") {
        return std::make_unique<Curtain>(client, id, extra);
    } else if (device_type == "Smart Lock") {
        return std::make_unique<Lock>(client, id, extra);
    } else {
        return std::make_unique<Device>(client, id, extra);
    }
}

nlohmann::json Device::status() {
    nlohmann::json response = client->get("devices/" + id + "/status");
    
    nlohmann::json result;
    if (response.contains("body")) {
        for (auto& [key, value] : response["body"].items()) {
            result[utils::to_snake_case(key)] = value;
        }
    }
    
    return result;
}

void Device::command(const std::string& action, const std::string& parameter) {
    nlohmann::json payload = {
        {"command_type", "command"},
        {"command", utils::to_camel_case(action)},
        {"parameter", parameter}
    };
    
    nlohmann::json camelized_payload = utils::camelize_json(payload);
    client->post("devices/" + id + "/commands", camelized_payload);
}

std::string Device::to_string() const {
    std::string name_str = type.empty() ? "Device" : type;
    // Remove spaces from name
    name_str.erase(std::remove(name_str.begin(), name_str.end(), ' '), name_str.end());
    return name_str + "(id=" + id + ")";
}

// Bot implementation
void Bot::turn(const std::string& state) {
    std::string lower_state = utils::to_lower(state);
    if (lower_state != "on" && lower_state != "off") {
        throw std::invalid_argument("State must be 'on' or 'off'");
    }
    command("turn_" + lower_state);
}

void Bot::press() {
    command("press");
}

void Bot::toggle() {
    nlohmann::json device_status = status();
    std::string power = device_status.value("power", "off");
    turn(power == "off" ? "on" : "off");
}

// Curtain implementation
Curtain::Curtain(std::shared_ptr<SwitchBotClient> client, const std::string& id,
                 const nlohmann::json& extra)
    : Device(client, id, extra), calibrated(false), grouped(false), master(false) {
    
    if (extra.contains("curtain_devices_ids") && extra["curtain_devices_ids"].is_array()) {
        for (const auto& curtain_id : extra["curtain_devices_ids"]) {
            curtain_ids.push_back(curtain_id.get<std::string>());
        }
    }
    if (extra.contains("calibrate")) {
        calibrated = extra["calibrate"].get<bool>();
    }
    if (extra.contains("group")) {
        grouped = extra["group"].get<bool>();
    }
    if (extra.contains("master")) {
        master = extra["master"].get<bool>();
    }
    if (extra.contains("open_direction")) {
        open_direction = extra["open_direction"].get<std::string>();
    }
}

// Lock implementation
void Lock::lock() {
    command("lock");
}

void Lock::unlock() {
    command("unlock");
}

void Lock::toggle() {
    nlohmann::json device_status = status();
    std::string lock_state = device_status.value("lock_state", "unlocked");
    
    if (lock_state != "unlocked" && lock_state != "locked") {
        throw std::runtime_error("Invalid lock state: " + lock_state);
    }
    
    if (lock_state == "unlocked") {
        lock();
    } else {
        unlock();
    }
}

} // namespace switchbot
