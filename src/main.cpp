#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <crow/app.h>
#include <nlohmann/json.hpp>

void signal_handler(const int signum) {
    // std::cout << "Interrupt signal (" << signum << ") received.\n";
    std::cout << "Exiting..." << std::endl;
    std::exit(signum);
}

int main(int argc, char **argv) {
    // Check for config file argument, exit if it's not there
    if (argc > 2) {
        std::cerr << "Usage: " << 0[argv] << " <config_file>" << std::endl;
        return 1;
    }

    std::string config_file_path = argc == 2 ? 1[argv] : "/etc/gh_wh_handler/config.json";

    // Open config file, exit if it fails
    std::ifstream config_file(config_file_path);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return 1;
    }

    // Parse config file, exit if it fails
    nlohmann::json config;
    try {
        config_file >> config;
    } catch (const std::exception &e) {
        std::cerr << "Failed to parse config.json: " << e.what() << std::endl;
        return 1;
    }

    // Set up variables from config file
    int port = config["port"];
    nlohmann::json &repos = config["repos"];
    nlohmann::json &tokens = config["tokens"];
    nlohmann::json &actions = config["actions"];

    // Close config file
    config_file.close();

    // Set up web server
    crow::SimpleApp app;

    // Set up route for updating files
    CROW_ROUTE(app, "/update-files")
        .methods("POST"_method)
        ([&repos, &tokens, &actions](const crow::request &req) {
            nlohmann::json payload;
            try {
                // Parse JSON payload from the request body, exit if it fails
                payload = nlohmann::json::parse(req.body);
            } catch (const std::exception &e) {
                std::cerr << "Error processing webhook: " << e.what() << std::endl;
                nlohmann::json response = {
                    {"status", 400},
                    {"error",  "Invalid JSON payload"}
                };
                return crow::response(400, response.dump());
            }

            try {
                std::string ref;
                std::string repo;
                bool is_private;
                std::string token;
                // Parse the payload
                try {
                    ref = payload["ref"];
                    if (size_t last_slash = ref.find_last_of('/'); last_slash != std::string::npos && last_slash + 1 < ref.length())
                        ref = ref.substr(last_slash + 1);
                    repo = payload["repository"]["full_name"];
                    is_private = payload["repository"]["private"];
                    token = std::string();
                    if (is_private) {
                        if (tokens.find(repo) == tokens.end()) {
                            printf("No token configured for private repo %s\n", repo.c_str());
                            nlohmann::json response = {
                                {"status", 403},
                                {"error",  "No token configured for private repo"}
                            };
                            return crow::response(403, response.dump());
                        }
                        token = tokens[repo];
                    }
                } catch (const std::exception &e) {
                    std::cerr << "Error parsing payload: " << e.what() << std::endl;
                    nlohmann::json response = {
                        {"status", 400},
                        {"error",  "Invalid JSON payload"}
                    };
                    return crow::response(400, response.dump());
                }

                printf("Received push to %s:%s (private: %s)\n", repo.c_str(), ref.c_str(), is_private ? "true" : "false");

                // Check if the repo is configured
                if (repos.find(repo) == repos.end()) {
                    printf("No webhook configured for %s\n", repo.c_str());
                    nlohmann::json response = {
                        {"status", 404},
                        {"error",  "No webhook configured for repo"}
                    };
                    return crow::response(404, response.dump());
                }

                // Check if the branch is configured
                nlohmann::json repo_data;
                bool is_valid_branch = false;
                for (auto c_repo = repos.begin(); c_repo != repos.end(); ++c_repo) {
                    if (const std::string &c_repo_name = c_repo.key(); c_repo_name != repo) continue;
                    if (c_repo.value()["branch"] != ref) continue;
                    is_valid_branch = true;
                    repo_data = c_repo.value();
                }

                if (!is_valid_branch) {
                    printf("No webhook configured for %s:%s\n", repo.c_str(), ref.c_str());
                    nlohmann::json response = {
                        {"status", 404},
                        {"error",  "No webhook configured for branch" + ref}
                    };
                    return crow::response(404, response.dump());
                }

                // Check if any files are configured
                if (repo_data["files"].empty()) {
                    printf("No files configured for %s:%s\n", repo.c_str(), ref.c_str());
                    nlohmann::json response = {
                        {"status", 404},
                        {"error",  "No files configured for branch" + ref}
                    };
                    return crow::response(404, response.dump());
                }

                // Get list with modified files
                std::vector<std::vector<std::string>> modified_files;
                for (auto &commit : payload["commits"]) {
                    for (auto &file : commit["modified"]) {
                        std::string file_path = file;
                        if (repos[repo]["files"].find(file_path) == repos[repo]["files"].end()) continue;
                        std::vector<std::string> file_data = {file_path, "modified"};
                        modified_files.push_back(file_data);
                    }
                    for (auto &file : commit["added"]) {
                        std::string file_path = file;
                        if (repos[repo]["files"].find(file_path) == repos[repo]["files"].end()) continue;
                        std::vector<std::string> file_data = {file_path, "added"};
                        modified_files.push_back(file_data);
                    }
                }

                // Download files
                nlohmann::json response = {
                    {"status", 200},
                    {"file_count", 0},
                    {"updated", nlohmann::json::array()}
                };
                for (std::vector<std::string> &file_data : modified_files) {
                    std::string file_path = file_data[0];

                    std::string path = repos[repo]["files"][file_path];
                    try {
                        std::filesystem::create_directories(path.substr(0, path.find_last_of('/')));
                    } catch (const std::exception &e) {
                        std::cerr << "Failed to create directories for " << path << ": " << e.what() << std::endl;
                        continue;
                    }

                    std::string command = "curl -s https://raw.githubusercontent.com/" + repo + "/" + ref + "/" + file_path + " -o " + path;
                    if (is_private) command += " -H 'Authorization: token " + token + "'";
                    std::system(command.c_str());
                    printf("%s %s\n", file_data[1] == "added" ? "Created" : "Updated", path.c_str());
                    response["file_count"] = response["file_count"].get<int>() + 1;
                    response["updated"].push_back(file_path);
                }

                // Run actions
                for (auto c_action_list = actions.begin(); c_action_list != actions.end(); ++c_action_list) {
                    if (const std::string &c_action_repo = c_action_list.key(); c_action_repo != repo) continue;
                    for (auto &action : c_action_list.value()) {
                        std::cout << "Executing action: " << action << std::endl;
                        std::string command = action;
                        int return_code = std::system(command.c_str());
                        std::ofstream log_file("/var/log/gh_wh_handler.log", std::ios_base::app);
                        time_t now = time(0);
                        if (return_code == 0) {
                            printf("Successfully executed action: %s\n", command.c_str());
                            log_file << now << " > Successfully executed action: " << command << std::endl;
                        } else {
                            printf("Failed to execute action: %s\n", command.c_str());
                            log_file << now << " > Failed to execute action: " << command << std::endl;
                        }
                    }
                }

                return crow::response(200, response.dump());
            } catch (const std::exception &e) {
                std::cerr << "Error processing webhook: " << e.what() << std::endl;
                return crow::response(500);
            }
        });

    std::signal(SIGINT, signal_handler);

    app.port(port).multithreaded().run();

    return 0;
}
