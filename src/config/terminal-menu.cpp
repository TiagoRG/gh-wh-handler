#include <iostream>
#include <fstream>
#include "config/terminal-menu.hpp"
#include "config/config-api.hpp"
#include "logger.hpp"

void print_menu() {
    std::cout << "1. Print menu" << std::endl;
    std::cout << "2. Preview current config" << std::endl;
    std::cout << "3. Set port" << std::endl;
    std::cout << "4. Add token" << std::endl;
    std::cout << "5. Remove token" << std::endl;
    std::cout << "6. Add run actions repo" << std::endl;
    std::cout << "7. Add run actions action" << std::endl;
    std::cout << "8. Remove run actions repo" << std::endl;
    std::cout << "9. Remove run actions action" << std::endl;
    std::cout << "10. Add update files repo" << std::endl;
    std::cout << "11. Add update files file" << std::endl;
    std::cout << "12. Add update files post-update" << std::endl;
    std::cout << "13. Remove update files repo" << std::endl;
    std::cout << "14. Remove update files file" << std::endl;
    std::cout << "15. Remove update files post-update" << std::endl;
    std::cout << "16. Exit and save changes" << std::endl;
    std::cout << "17. Exit and discard changes" << std::endl;
}

void TerminalMenu::open_menu(nlohmann::json config, std::string config_file_path) {
    Logger::warn("[Config] Config menu (no TUI available, using terminal only)");

    print_menu();

    while (true) {
        std::cout << ">>> ";

        int choice;
        std::cin >> choice;

        switch (choice) {
            case 1: {
                print_menu();
                break;
            }
            case 2: {
                Logger::info("[Config] Current config: ");
                Logger::code(config.dump(2));
                break;
            }
            case 3: {
                int port;
                std::cout << "Enter new port: ";
                std::cin >> port;
                config = ConfigApi::set_port(config, port);
                break;
            }
            case 4: {
                std::string repo, token;
                std::cout << "Enter repo name (owner/name): ";
                std::cin >> repo;
                if (config["tokens"].find(repo) != config["tokens"].end()) {
                    std::cout << "Token already exists for this repo. Do you want to update it? (y/N): ";
                    char update;
                    std::cin >> update;
                    if (update != 'y') {
                        break;
                    }
                }
                std::cout << "Enter token: ";
                std::cin >> token;
                config = ConfigApi::add_token(config, repo, token);
                break;
            }
            case 5: {
                std::string repo;
                std::cout << "Enter repo name (owner/name): ";
                std::cin >> repo;
                config = ConfigApi::remove_token(config, repo);
                break;
            }
            case 16: {
                Logger::info("[Config] Saving changes to config file: " + config_file_path);
                Logger::info("[Config] New config: ");
                std::ofstream config_file(config_file_path);
                Logger::code(config.dump(2));
                config_file << config.dump(2);
                config_file.close();
                return;
            }
            case 17: {
                std::cout << "Exiting without saving changes" << std::endl;
                return;
            }
            default:
                std::cout << "Invalid choice" << std::endl;
        }

    }
}

