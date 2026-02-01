#include "switchbot/scene.hpp"

namespace switchbot {

Scene::Scene(std::shared_ptr<SwitchBotClient> client, const std::string& id,
             const nlohmann::json& extra)
    : client(client), id(id) {
    
    if (extra.contains("scene_name")) {
        name = extra["scene_name"].get<std::string>();
    }
}

void Scene::execute() {
    client->post("scenes/" + id + "/execute", nlohmann::json());
}

std::string Scene::to_string() const {
    std::string name_str = name.empty() ? "Scene" : name;
    // Remove spaces from name
    name_str.erase(std::remove(name_str.begin(), name_str.end(), ' '), name_str.end());
    return name_str + "(id=" + id + ")";
}

} // namespace switchbot
