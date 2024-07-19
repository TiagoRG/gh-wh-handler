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
    if (argc < 2 || argc > 3) {
        std::cerr << "Usage: " << 0[argv] << " </path/to/config.json> [--config]" << std::endl;
        return 1;
    }

    std::time_t now = std::time(nullptr);
    std::tm *now_tm = std::localtime(&now);
    char time_buffer[80];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", now_tm);

    std::string config_file_path = 1[argv];
    std::string log_file_path = config_file_path.substr(0, config_file_path.find_last_of("/")) + "/gh-wh-handler_" + time_buffer + ".log";
    Logger::init(log_file_path);

    if (argc == 3 && std::string(argv[2]) == "--config") {
        Config::open_config_menu();
        return 0;
    }

    // Check if config file exists
    if (!std::filesystem::exists(config_file_path)) {
        Logger::warn("Config file does not exist, creating...");
        Config::create_config();
    }

    // Load configuration
    nlohmann::json config = Config::get_config(config_file_path);

    std::signal(SIGINT, signal_handler);

    // Start server
    Routes routes(config);

    return 0;
}
