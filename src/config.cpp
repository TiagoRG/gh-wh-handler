#include "config.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

#include "logger.hpp"

void Config::create_config(std::string config_file_path) {
    std::cout << "Creating config file" << std::endl;
    nlohmann::json config = {
        {"port", 65001},
        {"update-files", nlohmann::json::array()},
        {"run-scripts", nlohmann::json::array()},
        {"tokens", nlohmann::json::array()},
    };
    std::string path_to_config = config_file_path.substr(0, config_file_path.find_last_of('/'));
    if (!std::filesystem::exists(path_to_config)) {
        try {
            std::filesystem::create_directories(path_to_config);
        } catch (std::exception &e) {
            Logger::error("[Config] Error creating directory '" + path_to_config +"': " + std::string(e.what()));
            return;
        }
    }
    try {
        std::ofstream config_file(config_file_path);
        config_file << config.dump(2);
        config_file.close();
    } catch (std::exception& e) {
        Logger::fatal("[Config] Error creating config file: " + std::string(e.what()));
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
        Logger::fatal("Error loading config file: " + std::string(e.what()));
    }

    Logger::success("[Config] Loaded config file: " + config_file_path);
    Logger::info("[Config] Loaded config: ");
    Logger::code(config.dump(2));

    return config;
}

void Config::open_config_menu() {
    Logger::warn("[Config] Config menu not implemented yet");
}
