#include "endpoints/update-files.hpp"
#include <vector>

crow::response update_files(const nlohmann::json& config_update_files, const nlohmann::json& config_tokens, const crow::request& req) {
    nlohmann::json payload;
    try {
        payload = nlohmann::json::parse(req.body);
    } catch (nlohmann::json::parse_error& e) {
        std::cerr << "Error parsing payload: " << e.what() << std::endl;
        nlohmann::json response = {
            {"status", 400},
            {"error", "Error parsing payload"}
        };
        return crow::response(400, response.dump());
    }

    std::string ref;
    std::string repo;
    bool is_private;
    std::string token;

    try {
        ref = payload["ref"];
        if (size_t last_slash = ref.find_last_of('/'); last_slash != std::string::npos && last_slash + 1 < ref.length())
            ref = ref.substr(last_slash + 1);
        repo = payload["repository"]["full_name"];
        is_private = payload["repository"]["private"];
        if (is_private) {
            if (config_tokens.find(repo) == config_tokens.end()) {
                printf("No token configured for private repo %s\n", repo.c_str());
                nlohmann::json response = {
                    {"status", 403},
                    {"error",  "No token configured for private repo"}
                };
                return crow::response(403, response.dump());
            }
            token = config_tokens[repo];
        }
    } catch (nlohmann::json::out_of_range& e) {
        std::cerr << "Error parsing payload: " << e.what() << std::endl;
        nlohmann::json response = {
            {"status", 400},
            {"error", "Invalid JSON payload"}
        };
        return crow::response(400, response.dump());
    }

    printf("Received push to %s:%s (private: %s)\n", repo.c_str(), ref.c_str(), is_private ? "true" : "false");

    if (config_update_files.find(repo) == config_update_files.end()) {
        printf("No update-files webhook configuration for repo %s\n", repo.c_str());
        nlohmann::json response = {
            {"status", 404},
            {"error", "No update-files webhook configuration for repo"}
        };
        return crow::response(404, response.dump());
    }

    nlohmann::json config;
    bool found = false;
    for (auto c_repo = config_update_files.begin(); c_repo != config_update_files.end(); ++c_repo) {
        if (const std::string &c_repo_name = c_repo.key(); c_repo_name == repo) continue;
        if (c_repo.value()["branch"] != ref) continue;
        config = c_repo.value();
        found = true;
    }
    if (!found) {
        printf("No update-files webhook configuration for repo %s:%s\n", repo.c_str(), ref.c_str());
        nlohmann::json response = {
            {"status", 404},
            {"error", "No update-files webhook configuration for branch" + ref}
        };
        return crow::response(404, response.dump());
    }

    if (config["files"].empty()) {
        printf("No files configured for repo %s:%s\n", repo.c_str(), ref.c_str());
        nlohmann::json response = {
            {"status", 404},
            {"error", "No files configured for branch" + ref}
        };
        return crow::response(404, response.dump());
    }

    std::vector<std::vector<std::string>> modified_files;
    for (auto &commit : payload["commits"]) {
        for (auto &file : commit["added"]) {
            std::string file_path = file;
            if (config_update_files[repo]["files"].find(file_path) == config_update_files[repo]["files"].end()) continue;
            std::vector<std::string> file_info = {file_path, "added"};
            modified_files.push_back(file_info);
        }
        for (auto &file : commit["modified"]) {
            std::string file_path = file;
            if (config_update_files[repo]["files"].find(file_path) == config_update_files[repo]["files"].end()) continue;
            std::vector<std::string> file_info = {file_path, "modified"};
            modified_files.push_back(file_info);
        }
    }

    nlohmann::json response = {
        {"status", 200},
        {"message", "OK"},
        {"updated-files", nlohmann::json::array()}
    };
    for (auto &file : modified_files) {
        std::string remote_path = file[0];

        std::string local_path = config_update_files[repo]["files"][remote_path];
        try {
            std::filesystem::create_directories(local_path.substr(0, local_path.find_last_of('/')));
        } catch (const std::exception &e) {
            std::cerr << "Failed to create directories for " << local_path << ": " << e.what() << std::endl;
            continue;
        }

        std::string command = "curl -s https://raw.githubusercontent.com/" + repo + "/" + ref + "/" + remote_path + " -o " + local_path;
        if (is_private) command += " -H 'Authorization: token " + token + "'";
        std::system(command.c_str());
        printf("%s %s\n", file[1] == "added" ? "Created" : "Updated", local_path.c_str());
        response["file_count"] = response["file_count"].get<int>() + 1;
        response["updated"].push_back(remote_path);
    }

    for (auto &c_action : config_update_files[repo]["post-update"]) {
        std::string action = c_action.get<std::string>();
        printf("Running post-update action: %s\n", action.c_str());
        int return_code = std::system(action.c_str());
        std::ofstream log_file("/var/log/gh-wh-handler.log", std::ios_base::app);
        time_t now = time(0);
        if (return_code == 0) {
            printf("Post-update action %s ran successfully\n", action.c_str());
            log_file << ctime(&now) << "Post-update action " << action << " ran successfully\n";
        } else {
            printf("Post-update action %s failed with return code %d\n", action.c_str(), return_code);
            log_file << ctime(&now) << "Post-update action " << action << " failed with return code " << return_code << "\n";
        }
    }

    return crow::response(200, response.dump());
}

