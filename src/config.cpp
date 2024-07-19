#include "config.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

#include "logger.hpp"

void Config::create_config() {
    std::cout << "Creating config file" << std::endl;
    nlohmann::json config = {
        {"port", 65001},
        {"update-files", nlohmann::json::array()},
        {"run-scripts", nlohmann::json::array()},
        {"tokens", nlohmann::json::array()},
    };
    if (!std::filesystem::exists("/services/gh-wh-handler")) {
        try {
            std::filesystem::create_directories("/services/gh-wh-handler");
        } catch (std::exception &e) {
            Logger::error("[Config] Error creating directory '/services/gh-wh-handler/': " + std::string(e.what()));
            return;
        }
    }
    try {
        std::ofstream config_file("/services/gh-wh-handler/config.json");
        config_file << config.dump(2);
        config_file.close();
    } catch (std::exception& e) {
        Logger::error("[Config] Error creating config file: " + std::string(e.what()));
    }
}

nlohmann::json Config::get_config(std::string config_file_path) {
    Logger::info("[Config] Loading config file: " + config_file_path);

    nlohmann::json config;
    try {
        std::ifstream config_file(config_file_path);
        config_file >> config;
        config_file.close();
    } catch (std::exception& e) {
        Logger::error("Error loading config file: " + std::string(e.what()));
    }

    Logger::success("[Config] Loaded config file: " + config_file_path);
    Logger::info("[Config] Loaded config: ");
    Logger::code(config.dump(2));

    return config;
}

void Config::open_config_menu() {
    Logger::warn("[Config] Config menu not implemented yet");
}
