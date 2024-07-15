#include "config.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>

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
        } catch (std::exception& e) {
            std::cerr << "Error creating directory '/services/gh-wh-handler/': " << e.what() << std::endl;
            return;
        }
    }
    try {
        std::ofstream config_file("/services/gh-wh-handler/config.json");
        config_file << config.dump(2);
        config_file.close();
    } catch (std::exception& e) {
        std::cerr << "Error creating config file: " << e.what() << std::endl;
    }
}

nlohmann::json Config::get_config() {
    nlohmann::json config;
    try {
        std::ifstream config_file("/services/gh-wh-handler/config.json");
        config_file >> config;
        config_file.close();
    } catch (std::exception& e) {
        std::cerr << "Error loading config file: " << e.what() << std::endl;
    }

    return config;
}

void Config::open_config_menu() {
    std::cout << "Not implemented yet" << std::endl;
}
