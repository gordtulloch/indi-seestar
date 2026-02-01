#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "switchbot/switchbot_client.hpp"

namespace switchbot {

/**
 * Scene class for executing SwitchBot scenes
 */
class Scene {
public:
    /**
     * Constructor
     * @param client Shared pointer to SwitchBotClient
     * @param id Scene ID
     * @param extra Additional scene properties from API
     */
    Scene(std::shared_ptr<SwitchBotClient> client, const std::string& id,
          const nlohmann::json& extra = nlohmann::json());

    /**
     * Execute the scene
     */
    void execute();

    /**
     * Get string representation of scene
     */
    std::string to_string() const;

    // Getters
    std::string get_id() const { return id; }
    std::string get_name() const { return name; }

private:
    std::shared_ptr<SwitchBotClient> client;
    std::string id;
    std::string name;
};

} // namespace switchbot
