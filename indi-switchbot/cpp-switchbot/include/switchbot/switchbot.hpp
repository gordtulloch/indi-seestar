#pragma once

#include <string>
#include <memory>
#include <vector>
#include "switchbot/switchbot_client.hpp"
#include "switchbot/device.hpp"
#include "switchbot/remote.hpp"
#include "switchbot/scene.hpp"

namespace switchbot {

/**
 * Main SwitchBot API wrapper class
 */
class SwitchBot {
public:
    /**
     * Constructor
     * @param token SwitchBot API token
     * @param secret SwitchBot API secret
     */
    SwitchBot(const std::string& token, const std::string& secret);

    /**
     * Get all devices
     * @return Vector of unique pointers to Device objects
     */
    std::vector<std::unique_ptr<Device>> devices();

    /**
     * Get a specific device by ID
     * @param id Device ID
     * @return Unique pointer to Device
     * @throws std::runtime_error if device not found
     */
    std::unique_ptr<Device> device(const std::string& id);

    /**
     * Get all remotes
     * @return Vector of unique pointers to Remote objects
     */
    std::vector<std::unique_ptr<Remote>> remotes();

    /**
     * Get a specific remote by ID
     * @param id Remote ID
     * @return Unique pointer to Remote
     * @throws std::runtime_error if remote not found
     */
    std::unique_ptr<Remote> remote(const std::string& id);

    /**
     * Get all scenes
     * @return Vector of Scene objects
     */
    std::vector<Scene> scenes();

    /**
     * Get a specific scene by ID
     * @param id Scene ID
     * @return Scene object
     * @throws std::runtime_error if scene not found
     */
    Scene scene(const std::string& id);

    /**
     * Get the underlying client
     * @return Shared pointer to SwitchBotClient
     */
    std::shared_ptr<SwitchBotClient> get_client() const { return client; }

private:
    std::shared_ptr<SwitchBotClient> client;
};

} // namespace switchbot
