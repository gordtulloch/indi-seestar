#include "switchbot/remote.hpp"
#include "switchbot/utils.hpp"
#include <stdexcept>

namespace switchbot {

Remote::Remote(std::shared_ptr<SwitchBotClient> client, const std::string& id,
               const nlohmann::json& extra)
    : client(client), id(id) {
    
    if (extra.contains("device_name")) {
        name = extra["device_name"].get<std::string>();
    }
    if (extra.contains("remote_type")) {
        type = extra["remote_type"].get<std::string>();
    }
    if (extra.contains("hub_device_id")) {
        hub_id = extra["hub_device_id"].get<std::string>();
    }
}

std::unique_ptr<Remote> Remote::create(std::shared_ptr<SwitchBotClient> client,
                                      const std::string& id,
                                      const nlohmann::json& extra) {
    std::string remote_type;
    if (extra.contains("remote_type")) {
        remote_type = extra["remote_type"].get<std::string>();
    }
    
    if (remote_type == "Others") {
        return std::make_unique<OtherRemote>(client, id, extra);
    } else {
        return std::make_unique<SupportedRemote>(client, id, extra);
    }
}

void Remote::command(const std::string& action, const std::string& parameter, bool customize) {
    std::string command_type = customize ? "customize" : "command";
    std::string command_action = customize ? action : utils::to_camel_case(action);
    
    nlohmann::json payload = {
        {"command_type", command_type},
        {"command", command_action},
        {"parameter", parameter}
    };
    
    nlohmann::json camelized_payload = utils::camelize_json(payload);
    client->post("devices/" + id + "/commands", camelized_payload);
}

std::string Remote::to_string() const {
    std::string name_str = type.empty() ? "Remote" : type;
    // Remove spaces from name
    name_str.erase(std::remove(name_str.begin(), name_str.end(), ' '), name_str.end());
    return name_str + "(id=" + id + ")";
}

// SupportedRemote implementation
void SupportedRemote::turn(const std::string& state) {
    std::string lower_state = utils::to_lower(state);
    if (lower_state != "on" && lower_state != "off") {
        throw std::invalid_argument("State must be 'on' or 'off'");
    }
    command("turn_" + lower_state);
}

// OtherRemote implementation
void OtherRemote::command(const std::string& action, const std::string& parameter, bool customize) {
    Remote::command(action, parameter, true);
}

} // namespace switchbot
