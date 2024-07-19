#include "endpoints/update-files.hpp"
#include <vector>

#include "logger.hpp"

crow::response update_files(const nlohmann::json& config_update_files, const nlohmann::json& config_tokens, const crow::request& req) {
    nlohmann::json payload;
    try {
        payload = nlohmann::json::parse(req.body);
    } catch (nlohmann::json::parse_error& e) {
        Logger::error("[/update-files] Error parsing payload: " + std::string(e.what()));
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
                Logger::warn("[/update-files] No token configured for private repo " + repo);
                nlohmann::json response = {
                    {"status", 403},
                    {"error",  "No token configured for private repo"}
                };
                return crow::response(403, response.dump());
            }
            token = config_tokens[repo];
        }
    } catch (nlohmann::json::out_of_range& e) {
        Logger::error("[/update-files] Invalid JSON payload: " + std::string(e.what()));
        nlohmann::json response = {
            {"status", 400},
            {"error", "Invalid JSON payload"}
        };
        return crow::response(400, response.dump());
    }

    Logger::info("[/update-files] Received push to " + repo + ":" + ref + " (private: " + (is_private ? "true" : "false") + ")");

    if (config_update_files.find(repo) == config_update_files.end()) {
        Logger::warn("[/update-files] No update-files webhook configuration for repo " + repo);
        nlohmann::json response = {
            {"status", 404},
            {"error", "No update-files webhook configuration for repo"}
        };
        return crow::response(404, response.dump());
    }

    Logger::info("[/update-files] Found update-files webhook configuration for repo " + repo);

    nlohmann::json config;
    bool found = false;
    for (auto c_repo = config_update_files.begin(); c_repo != config_update_files.end(); ++c_repo) {
        if (const std::string &c_repo_name = c_repo.key(); c_repo_name == repo) continue;
        if (c_repo.value()["branch"] != ref) continue;
        config = c_repo.value();
        found = true;
    }
    if (!found) {
        Logger::warn("[/update-files] No update-files webhook configuration for branch " + ref);
        nlohmann::json response = {
            {"status", 404},
            {"error", "No update-files webhook configuration for branch" + ref}
        };
        return crow::response(404, response.dump());
    }

    Logger::info("[/update-files] Found update-files webhook configuration for branch " + ref);

    if (config["files"].empty()) {
        Logger::warn("[/update-files] No files configured for repo " + repo + ":" + ref);
        nlohmann::json response = {
            {"status", 404},
            {"error", "No files configured for repo" + repo + ":" + ref}
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

    Logger::info("[/update-files] Found " + std::to_string(modified_files.size()) + " files to update for repo " + repo + ":" + ref);

    if (modified_files.empty()) {
        Logger::info("[/update-files] No files to update for repo " + repo + ":" + ref);
        nlohmann::json response = {
            {"status", 200},
            {"message", "No files to update"}
        };
        return crow::response(200, response.dump());
    }

    Logger::info("[/update-files] Updating " + std::to_string(modified_files.size()) + " files for repo " + repo + ":" + ref);

    nlohmann::json response = {
        {"status", 200},
        {"message", "OK"},
        {"file_count", 0},
        {"updated-files", nlohmann::json::array()}
    };
    for (auto &file : modified_files) {
        std::string remote_path = file[0];

        std::string local_path = config_update_files[repo]["files"][remote_path];
        try {
            std::filesystem::create_directories(local_path.substr(0, local_path.find_last_of('/')));
        } catch (const std::exception &e) {
            Logger::error("[/update-files] Failed to create directories for " + local_path + ": " + std::string(e.what()));
            continue;
        }

        std::string command = "curl -s https://raw.githubusercontent.com/" + repo + "/" + ref + "/" + remote_path + " -o " + local_path;
        if (is_private) command += " -H 'Authorization: token " + token + "'";
        std::system(command.c_str());
        Logger::info("[/update-files] " + std::string(file[1] == "added" ? "Created" : "Updated") + " " + local_path);
        response["file_count"] = response["file_count"].get<int>() + 1;
        response["updated-files"].push_back(remote_path);
    }

    Logger::success("[/update-files] Finished updating " + std::to_string(modified_files.size()) + " files for repo " + repo + ":" + ref);
    Logger::info("[/update-files] Running post-update actions for repo " + repo + ":" + ref);

    for (auto &c_action : config_update_files[repo]["post-update"]) {
        std::string action = c_action.get<std::string>();
        Logger::info("[/update-files] Running post-update action: " + action);
        int return_code = std::system(action.c_str());
        if (return_code == 0) {
            Logger::success("[/update-files] Post-update action " + action + " ran successfully");
        } else {
            Logger::error("[/update-files] Post-update action " + action + " failed with return code " + std::to_string(return_code));
        }
    }

    Logger::success("[/update-files] Finished running post-update actions for repo " + repo + ":" + ref);

    return crow::response(200, response.dump());
}

