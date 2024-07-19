#include "endpoints/update-files.hpp"
#include <cmath>
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

    if (payload.find("ref") == payload.end() || payload.find("repository") == payload.end() ||
        payload["repository"].find("full_name") == payload["repository"].end() || payload["repository"].find("private") == payload["repository"].end() ||
        payload.find("commits") == payload.end() || !payload["commits"].is_array() || payload["commits"].empty()){
        Logger::error("[/update-files] Invalid JSON payload");
        nlohmann::json response = {
            {"status", 400},
            {"error", "Invalid JSON payload"}
        };
        return crow::response(400, response.dump());
    }

    std::string ref = payload["ref"];
    if (size_t last_slash = ref.find_last_of('/'); last_slash != std::string::npos && last_slash + 1 < ref.length())
        ref = ref.substr(last_slash + 1);
    std::string repo = payload["repository"]["full_name"];
    bool is_private = payload["repository"]["private"];
    std::string token;
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
        if (c_repo.key() != repo) continue;
        if (c_repo.value().find("branch") == c_repo.value().end() || c_repo.value().find("files") == c_repo.value().end()) {
            Logger::error("[/update-files] Invalid update-files configuration found for repo " + c_repo.key());
            nlohmann::json response = {
                {"status", 500},
                {"error", "Invalid update-files configuration"}
            };
            return crow::response(500, response.dump());
        }
        if (c_repo.value()["branch"] != ref) continue;
        config = c_repo.value();
        found = true;
    }
    if (!found) {
        Logger::warn("[/update-files] Ignoring push to non-configured branch" + ref + " on repo " + repo);
        nlohmann::json response = {
            {"status", 200},
            {"message", "Ignoring push to non-configured branch" + ref}
        };
        return crow::response(404, response.dump());
    }

    Logger::info("[/update-files] Found update-files webhook configuration for branch " + ref);

    if (config["files"].empty()) {
        Logger::warn("[/update-files] No files configured for repo " + repo + ":" + ref);
        nlohmann::json response = {
            {"status", 200},
            {"message", "No files configured for repo " + repo + ":" + ref}
        };
        return crow::response(200, response.dump());
    }

    std::vector<std::vector<std::string>> modified_files;
    for (auto &commit : payload["commits"]) {
        if (commit.find("added") == commit.end() || commit.find("modified") == commit.end()) {
            Logger::error("[/update-files] Invalid JSON payload");
            nlohmann::json response = {
                {"status", 400},
                {"error", "Invalid JSON payload"}
            };
            return crow::response(400, response.dump());
        }
        for (auto &file : commit["added"]) {
            std::string file_path = file;
            if (config["files"].find(file_path) == config["files"].end()) continue;
            std::vector<std::string> file_info = {file_path, "added"};
            modified_files.push_back(file_info);
        }
        for (auto &file : commit["modified"]) {
            std::string file_path = file;
            if (config["files"].find(file_path) == config["files"].end()) continue;
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
        {"updated-file-count", 0},
        {"updated-files", {
            {"successful", nlohmann::json::array()},
            {"failed", nlohmann::json::array()}
        }},
        {"post-update", {
            {"successful", nlohmann::json::array()},
            {"failed", nlohmann::json::array()}
        }}
    };
    for (auto &file : modified_files) {
        std::string remote_path = file[0];

        std::string local_path = config["files"][remote_path];
        if (local_path.find_last_of('/') != std::string::npos)
            try {
                std::filesystem::create_directories(local_path.substr(0, local_path.find_last_of('/')));
            } catch (const std::exception &e) {
                Logger::error("[/update-files] Failed to create directories for " + local_path + ": " + std::string(e.what()));
                response["updated-files"]["failed"].push_back(local_path);
                continue;
            }

        std::string command = "curl -s https://raw.githubusercontent.com/" + repo + "/" + ref + "/" + remote_path + " -o " + local_path;
        if (is_private) command += " -H 'Authorization: token " + token + "'";
        int ret = std::system(command.c_str());
        if (ret != 0) {
            Logger::error("[/update-files] Failed to update " + local_path);
            response["updated-files"]["failed"].push_back(local_path);
            continue;
        }
        Logger::info("[/update-files] " + std::string(file[1] == "added" ? "Created" : "Updated") + " " + local_path);
        response["updated-file-count"] = response["updated-file-count"].get<int>() + 1;
        response["updated-files"]["successful"].push_back(local_path);
    }

    Logger::success("[/update-files] Finished updating " + std::to_string(modified_files.size()) + " files for repo " + repo + ":" + ref);

    if (config.find("post-update") == config.end()) {
        Logger::info("[/update-files] No post-update actions configured for repo " + repo + ":" + ref);
        return crow::response(200, response.dump());
    }

    nlohmann::json post_update_actions = config["post-update"];

    Logger::info("[/update-files] Running post-update actions for repo " + repo + ":" + ref);

    for (const auto &action : post_update_actions) {
        if (action.find("name") == action.end() || action.find("command") == action.end()) {
            Logger::error("[/update-files] Invalid post-update configuration for repo " + repo);
            nlohmann::json e_response = {
                {"status", 500},
                {"error", "Invalid action configuration"},
                {"updated-files", response["updated-files"]},
                {"post-update", response["post-update"]}
            };
            return crow::response(500, e_response.dump());
        }
        std::string action_name = action["name"];
        std::string action_command = action["command"];

        Logger::info("[/update-files] Running post-update action '" + action_name + "'");

        int ret = std::system(action_command.c_str());
        if (ret == 0) {
            Logger::info("[/update-files] Action " + action_name + " completed successfully");
            response["post-update"]["successful"].push_back(action_name);
        } else {
            Logger::error("[/update-files] Action " + action_name + " failed with exit code " + std::to_string(ret));
            response["post-update"]["failed"].push_back(action_name);
        }
    }

    Logger::success("[/update-files] Finished running post-update actions for repo " + repo + ":" + ref);

    return crow::response(200, response.dump());
}

