#pragma once

#include <string>
#include <memory>
#include <nlohmann/json.hpp>
#include "switchbot/switchbot_client.hpp"

namespace switchbot {

/**
 * Base class for all SwitchBot remotes (infrared devices)
 */
class Remote {
public:
    /**
     * Constructor
     * @param client Shared pointer to SwitchBotClient
     * @param id Remote ID
     * @param extra Additional remote properties from API
     */
    Remote(std::shared_ptr<SwitchBotClient> client, const std::string& id,
           const nlohmann::json& extra = nlohmann::json());

    virtual ~Remote() = default;

    /**
     * Factory method to create appropriate remote type
     * @param client Shared pointer to SwitchBotClient
     * @param id Remote ID
     * @param extra Additional remote properties from API
     * @return Unique pointer to Remote
     */
    static std::unique_ptr<Remote> create(std::shared_ptr<SwitchBotClient> client,
                                         const std::string& id,
                                         const nlohmann::json& extra);

    /**
     * Send command to remote
     * @param action Command action name
     * @param parameter Optional command parameter
     * @param customize Whether this is a custom command
     */
    virtual void command(const std::string& action, const std::string& parameter = "default",
                        bool customize = false);

    /**
     * Get string representation of remote
     */
    virtual std::string to_string() const;

    // Getters
    std::string get_id() const { return id; }
    std::string get_name() const { return name; }
    std::string get_type() const { return type; }
    std::string get_hub_id() const { return hub_id; }

protected:
    std::shared_ptr<SwitchBotClient> client;
    std::string id;
    std::string name;
    std::string type;
    std::string hub_id;
};

/**
 * Supported remote devices (standard IR remotes like TV, AC, etc.)
 */
class SupportedRemote : public Remote {
public:
    using Remote::Remote;

    /**
     * Turn remote on or off
     * @param state "on" or "off"
     */
    void turn(const std::string& state);
};

/**
 * Other/custom remote devices
 */
class OtherRemote : public Remote {
public:
    using Remote::Remote;

    /**
     * Send custom command to remote
     * @param action Command action name
     * @param parameter Optional command parameter
     */
    void command(const std::string& action, const std::string& parameter = "default", bool customize = true);
};

} // namespace switchbot
