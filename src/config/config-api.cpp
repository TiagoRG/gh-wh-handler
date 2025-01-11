#include "config/config-api.hpp"
#include "logger.hpp"

bool ConfigApi::set_port(nlohmann::json config, int port) {
    Logger::info("[Config] Setting port to " + std::to_string(port));
    config["port"] = port;
    return true;
}

bool ConfigApi::add_token(nlohmann::json config, std::string repo, std::string token) {
    Logger::info("[Config] Adding token for repo " + repo);
    if (config["tokens"].find(repo) != config["tokens"].end()) {
        Logger::warn("Token already exists for this repo.");
        return false;
    }
    config["tokens"][repo] = token;
    return true;
}

bool ConfigApi::remove_token(nlohmann::json config, std::string repo) {
    Logger::info("[Config] Removing token for repo " + repo);
    if (config["tokens"].find(repo) == config["tokens"].end()) {
        Logger::warn("Token does not exist for this repo.");
        return false;
    }
    config["tokens"].erase(repo);
    return true;
}
