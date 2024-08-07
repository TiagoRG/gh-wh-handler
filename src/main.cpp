#include <iostream>
#include <cstdio>
#include <string>
#include <nlohmann/json.hpp>

#include "logger.hpp"
#include "routes.hpp"
#include "config.hpp"

void signal_handler(const int signum) {
    // std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "Exiting..." << std::endl;
    std::exit(signum);
}

int main(int argc, char **argv) {
    // Check for config file argument, exit if it's not there
    if (argc < 3 || argc > 4) {
        std::cerr << "Usage: " << 0[argv] << " </path/to/config.json> </path/to/logs_dir> [--config]" << std::endl;
        return 1;
    }

    std::time_t now = std::time(nullptr);
    std::tm *now_tm = std::localtime(&now);
    char time_buffer[80];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", now_tm);

    std::string config_file_path = 1[argv];
    std::string logs_dir = 2[argv];

    Logger::init(logs_dir);

    if (argc == 4 && std::string(argv[3]) == "--config") {
        Config::open_config_menu();
        return 0;
    }

    // Check if config file exists
    if (!std::filesystem::exists(config_file_path)) {
        Logger::warn("Config file does not exist, creating...");
        Config::create_config(config_file_path);
    }

    // Load configuration
    nlohmann::json config = Config::get_config(config_file_path);

    std::signal(SIGINT, signal_handler);

    // Start server
    Routes routes(config);

    return 0;
}
