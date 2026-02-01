#include "switchbot/switchbot.hpp"
#include <random>
#include <sstream>
#include <iomanip>
#include <stdexcept>

namespace switchbot {

// Generate a simple UUID-like nonce
static std::string generate_nonce() {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(0, 15);
    
    const char* hex_chars = "0123456789abcdef";
    std::stringstream ss;
    
    for (int i = 0; i < 32; ++i) {
        if (i == 8 || i == 12 || i == 16 || i == 20) {
            ss << '-';
        }
        ss << hex_chars[dis(gen)];
    }
    
    return ss.str();
}

SwitchBot::SwitchBot(const std::string& token, const std::string& secret)
    : client(std::make_shared<SwitchBotClient>(token, secret, generate_nonce())) {
}

std::vector<std::unique_ptr<Device>> SwitchBot::devices() {
    nlohmann::json response = client->get("devices");
    std::vector<std::unique_ptr<Device>> result;
    
    if (response.contains("body") && response["body"].contains("device_list")) {
        for (const auto& device_data : response["body"]["device_list"]) {
            std::string device_id = device_data.value("device_id", "");
            if (!device_id.empty()) {
                result.push_back(Device::create(client, device_id, device_data));
            }
        }
    }
    
    return result;
}

std::unique_ptr<Device> SwitchBot::device(const std::string& id) {
    // Currently, SwitchBot API does not support to retrieve device info
    // without getting all device list. Therefore, we query all devices first,
    // then return the matching device
    auto all_devices = devices();
    for (auto& dev : all_devices) {
        if (dev->get_id() == id) {
            return std::move(dev);
        }
    }
    
    throw std::runtime_error("Unknown device " + id);
}

std::vector<std::unique_ptr<Remote>> SwitchBot::remotes() {
    nlohmann::json response = client->get("devices");
    std::vector<std::unique_ptr<Remote>> result;
    
    if (response.contains("body") && response["body"].contains("infrared_remote_list")) {
        for (const auto& remote_data : response["body"]["infrared_remote_list"]) {
            std::string remote_id = remote_data.value("device_id", "");
            if (!remote_id.empty()) {
                result.push_back(Remote::create(client, remote_id, remote_data));
            }
        }
    }
    
    return result;
}

std::unique_ptr<Remote> SwitchBot::remote(const std::string& id) {
    auto all_remotes = remotes();
    for (auto& rem : all_remotes) {
        if (rem->get_id() == id) {
            return std::move(rem);
        }
    }
    
    throw std::runtime_error("Unknown remote " + id);
}

std::vector<Scene> SwitchBot::scenes() {
    nlohmann::json response = client->get("scenes");
    std::vector<Scene> result;
    
    if (response.contains("body") && response["body"].is_array()) {
        for (const auto& scene_data : response["body"]) {
            std::string scene_id = scene_data.value("scene_id", "");
            if (!scene_id.empty()) {
                result.emplace_back(client, scene_id, scene_data);
            }
        }
    }
    
    return result;
}

Scene SwitchBot::scene(const std::string& id) {
    auto all_scenes = scenes();
    for (const auto& scn : all_scenes) {
        if (scn.get_id() == id) {
            return scn;
        }
    }
    
    throw std::runtime_error("Unknown scene " + id);
}

} // namespace switchbot
