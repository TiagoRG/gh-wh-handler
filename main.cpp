#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <crow/app.h>
#include <nlohmann/json.hpp>

int main(int argc, char **argv) {
    // Check for config file argument, exit if it's not there
    if (argc != 2) {
        std::cerr << "Usage: " << 0[argv] << " <config_file>" << std::endl;
        return 1;
    }

    // Open config file, exit if it fails
    std::ifstream config_file(1[argv]);
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

    // Close config file
    config_file.close();

    // Set up web server
    crow::SimpleApp app;

    // Set up route for updating files
    CROW_ROUTE(app, "/update-files")
        .methods("POST"_method)
        ([&repos, &tokens](const crow::request &req) {
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
                // Parse the payload
                std::string ref = payload["ref"];
                size_t last_slash = ref.find_last_of('/');
                if (last_slash != std::string::npos && last_slash + 1 < ref.length())
                    ref = ref.substr(last_slash + 1);
                std::string repo = payload["repository"]["full_name"];
                bool is_private = payload["repository"]["private"];
                std::string token = std::string();
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
                    std::string c_repo_name = c_repo.key();
                    if (c_repo_name != repo) continue;
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

                // Download files
                nlohmann::json response = {
                    {"status", 200},
                    {"file_count", 0},
                    {"updated", nlohmann::json::array()}
                };
                for (auto &commit : payload["commits"]) {
                    for (auto &file : commit["modified"]) {
                        std::string file_path = file;
                        if (repos[repo]["files"].find(file_path) == repos[repo]["files"].end()) continue;

                        std::string path = repos[repo]["files"][file_path];
                        try {
                            std::filesystem::create_directories(path.substr(0, path.find_last_of('/')));
                        } catch (const std::exception &e) {
                            std::cerr << "Failed to create directories for " << path << ": " << e.what() << std::endl;
                            continue;
                        }

                        std::string command = "curl -s https://raw.githubusercontent.com/" + repo + "/" + ref + "/" + file_path + " -o " + path;
                        if (is_private)
                            command += " -H 'Authorization: token " + token + "'";
                        std::system(command.c_str());
                        printf("Updated %s\n", path.c_str());
                        response["file_count"] = response["file_count"].get<int>() + 1;
                        response["updated"].push_back(file_path);
                    }
                }

                return crow::response(200, response.dump());
            } catch (const std::exception &e) {
                std::cerr << "Error processing webhook: " << e.what() << std::endl;
                return crow::response(500);
            }
        });

    app.port(port).multithreaded().run();
}
