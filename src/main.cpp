#include <iostream>
#include <cstdio>
#include <string>
#include <nlohmann/json.hpp>

#include "routes.hpp"
#include "config.hpp"

void signal_handler(const int signum) {
    // std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "Exiting..." << std::endl;
    std::exit(signum);
}

int main(int argc, char **argv) {
    // Check for config file argument, exit if it's not there
    if (argc < 2 || argc > 2) {
        std::cerr << "Usage: " << 0[argv] << " <config | /path/to/config.json>" << std::endl;
        return 1;
    }

    if (std::string(argv[1]) == "config") {
        return 0;
    }

    std::string config_file_path = 1[argv];

    // Check if config file exists
    if (!std::filesystem::exists(config_file_path)) {
        std::cerr << "Config file does not exist, creating..." << std::endl;
        Config::create_config();
    }

    // Load configuration
    nlohmann::json config = Config::get_config();

    std::signal(SIGINT, signal_handler);

    // Start server
    Routes routes(config);

    return 0;
}
