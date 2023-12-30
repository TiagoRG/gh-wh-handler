#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <crow/app.h>
#include <nlohmann/json.hpp>

int main(int argc, char **argv) {

    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <config_file>" << std::endl;
        return 1;
    }

    std::ifstream config_file(argv[1]);
    if (!config_file.is_open()) {
        std::cerr << "Failed to open config.json" << std::endl;
        return 1;
    }

    nlohmann::json config;
    try {
        config_file >> config;
    } catch (const std::exception &e) {
        std::cerr << "Failed to parse config.json: " << e.what() << std::endl;
        return 1;
    }

    int port = config["port"];
    nlohmann::json &repos = config["repos"];
    nlohmann::json &tokens = config["tokens"];

    crow::SimpleApp app;
    CROW_ROUTE(app, "/github-webhook")
        .methods("POST"_method)
        ([&repos, &tokens](const crow::request &req) {
            try {
                // Parse JSON payload from the request body
                nlohmann::json payload = nlohmann::json::parse(req.body);
                nlohmann::json response;

                std::string ref = payload["ref"];
                size_t last_slash = ref.find_last_of('/');
                if (last_slash != std::string::npos && last_slash + 1 < ref.length())
                    ref = ref.substr(last_slash + 1);
                std::string repo = payload["repository"]["full_name"];
                bool is_private = payload["repository"]["private"];
                if (is_private && tokens.find(repo) == tokens.end()) {
                    printf("No token configured for private repo %s\n", repo.c_str());
                    response["status"] = 403;
                    response["error"] = "No token configured for private repo";
                    return crow::response(403, response.dump());
                }
                std::string token = tokens[repo];

                printf("Received push to %s:%s (private: %s)\n", repo.c_str(), ref.c_str(), is_private ? "true" : "false");

                if (repos.find(repo) == repos.end()) {
                    printf("No webhook configured for %s\n", repo.c_str());
                    response["status"] = 404;
                    response["error"] = "No webhook configured for repo";
                    return crow::response(404, response.dump());
                }

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
                    response["status"] = 404;
                    response["error"] = "No webhook configured for branch" + ref;
                    return crow::response(404, response.dump());
                }

                if (repo_data["files"].empty()) {
                    printf("No files configured for %s:%s\n", repo.c_str(), ref.c_str());
                    response["status"] = 404;
                    response["error"] = "No files configured for branch " + ref;
                    return crow::response(404, response.dump());
                }

                response["updated"] = nlohmann::json::array();
                for (auto &commit : payload["commits"]) {
                    for (auto &file : commit["modified"]) {
                        std::string file_path = file;
                        if (repos[repo]["files"].find(file_path) == repos[repo]["files"].end()) continue;
                        std::string path = repos[repo]["files"][file_path];
                        std::string create_dir = "mkdir -p $(dirname " + path + ")";
                        std::string command = "curl -s https://raw.githubusercontent.com/" + repo + "/" + ref + "/" + file_path + " -H 'Authorization: token " + token + "' -o " + path;
                        std::system(create_dir.c_str());
                        std::system(command.c_str());
                        printf("Updated %s\n", path.c_str());
                        response["updated"].push_back(file_path);
                    }
                }
                response["status"] = 200;

                return crow::response(200, response.dump());
            } catch (const std::exception &e) {
                std::cerr << "Error processing webhook: " << e.what() << std::endl;
                return crow::response(500);
            }
        });

    app.port(port).multithreaded().run();
}
