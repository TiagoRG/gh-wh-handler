#include "endpoints/run-actions.hpp"

#include "logger.hpp"

crow::response run_actions(const nlohmann::json &config_run_actions, const crow::request &req) {
    nlohmann::json payload;
    try {
        payload = nlohmann::json::parse(req.body);
    } catch (nlohmann::json::parse_error& e) {
        Logger::error("[/run-actions] Error parsing payload: " + std::string(e.what()));
        nlohmann::json response = {
            {"status", 400},
            {"error", "Error parsing payload"}
        };
        return crow::response(400, response.dump());
    }

    if (payload.find("ref") == payload.end() || payload.find("repository") == payload.end() || payload["repository"].find("full_name") == payload["repository"].end()) {
        Logger::error("[/run-actions] Invalid JSON payload");
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

    Logger::info("[/run-actions] Received push to " + repo + ":" + ref);

    if (config_run_actions.find(repo) == config_run_actions.end()) {
        Logger::warn("[/run-actions] No run-actions configuration for repo " + repo);
        nlohmann::json response = {
            {"status", 404},
            {"error", "No run-actions configuration for repo"}
        };
        return crow::response(404, response.dump());
    }

    Logger::info("[/run-actions] Found run-actions configuration for repo " + repo);

    nlohmann::json config;
    bool found = false;
    for (auto c_repo = config_run_actions.begin(); c_repo != config_run_actions.end(); ++c_repo) {
        if (c_repo.key() != repo) continue;
        if (c_repo.value().find("branch") == c_repo.value().end() || c_repo.value().find("actions") == c_repo.value().end()) {
            Logger::error("[/run-actions] Invalid run-actions configuration found for repo " + c_repo.key());
            nlohmann::json response = {
                {"status", 500},
                {"error", "Invalid update-files configuration"}
            };
            return crow::response(500, response.dump());
        }
        if (c_repo.value()["branch"].get<std::string>() != ref) continue;
        Logger::info("[/run-actions] Found run-actions configuration for branch " + ref);
        config = c_repo.value();
        found = true;
    }
    if (!found) {
        Logger::info("[/run-actions] Ignoring push to non-configured branch " + ref + " on repo " + repo);
        nlohmann::json response = {
            {"status", 200},
            {"message", "Ignoring push to non-configured branch" + ref}
        };
        return crow::response(200, response.dump());
    }

    std::string config_branch = config["branch"];
    nlohmann::json config_actions = config["actions"];

    if (config_actions.empty()) {
        Logger::info("[/run-actions] No actions configured for " + repo + " on branch " + ref);
        nlohmann::json response = {
            {"status", 200},
            {"message", "No actions configured for " + repo + " on branch " + ref}
        };
        return crow::response(200, response.dump());
    }

    Logger::info("[/run-actions] Running actions for " + repo + " on branch " + ref);

    nlohmann::json response = {
        {"status", 200},
        {"message", "OK"},
        {"successful-actions", nlohmann::json::array()},
        {"failed-actions", nlohmann::json::array()}
    };
    for (const auto &action : config_actions) {
        if (action.find("name") == action.end() || action.find("command") == action.end()) {
            Logger::error("[/run-actions] Invalid action configuration for repo " + repo);
            nlohmann::json e_response = {
                {"status", 500},
                {"error", "Invalid action configuration"},
                {"successful-actions", response["successful-actions"]},
                {"failed-actions", response["failed-actions"]}
            };
            return crow::response(500, e_response.dump());
        }
        std::string action_name = action["name"];
        std::string action_command = action["command"];

        Logger::info("[/run-actions] Running action '" + action_name + "'");

        int ret = std::system(action_command.c_str());
        if (ret == 0) {
            Logger::info("[/run-actions] Action " + action_name + " completed successfully");
            response["successful-actions"].push_back(action_name);
        } else {
            Logger::error("[/run-actions] Action " + action_name + " failed with exit code " + std::to_string(ret));
            response["failed-actions"].push_back(action_name);
        }
    }

    Logger::success("[/run-actions] Finished running actions for " + repo + " on branch " + ref);

    return crow::response(200, response.dump());
}

