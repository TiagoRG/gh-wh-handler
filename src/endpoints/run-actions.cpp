#include "endpoints/run-actions.hpp"

#include "logger.hpp"

crow::response run_actions(const nlohmann::json &run_actions, const nlohmann::json &tokens, const crow::request &req) {
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

    std::string ref;
    std::string repo;

    try {
        ref = payload["ref"];
        if (size_t last_slash = ref.find_last_of('/'); last_slash != std::string::npos && last_slash + 1 < ref.length())
            ref = ref.substr(last_slash + 1);
        repo = payload["repository"]["full_name"];
    } catch (nlohmann::json::out_of_range& e) {
        Logger::error("[/run-actions] Invalid JSON payload: " + std::string(e.what()));
        nlohmann::json response = {
            {"status", 400},
            {"error", "Invalid JSON payload"}
        };
        return crow::response(400, response.dump());
    }

    Logger::info("[/run-actions] Received push to " + repo + ":" + ref);

    if (run_actions.find(repo) == run_actions.end()) {
        Logger::warn("[/run-actions] No run-actions webhook configuration for repo " + repo);
        nlohmann::json response = {
            {"status", 404},
            {"error", "No run-actions webhook configuration for repo"}
        };
        return crow::response(404, response.dump());
    }

    Logger::info("[/run-actions] Found run-actions webhook configuration for repo " + repo);

    nlohmann::json config;
    std::string config_branch;
    nlohmann::json config_actions;
    try {
        config = run_actions[repo];
        config_branch = config["branch"];
        config_actions = config["actions"];
    } catch (nlohmann::json::out_of_range& e) {
        Logger::error("[/run-actions] Invalid run-actions configuration for repo " + repo + ": " + std::string(e.what()));
        nlohmann::json response = {
            {"status", 500},
            {"error", "Invalid run-actions configuration"}
        };
        return crow::response(500, response.dump());
    }

    if (ref != config_branch) {
        Logger::info("[/run-actions] Ignoring push to " + repo + " on branch " + ref);
        nlohmann::json response = {
            {"status", 200},
            {"message", "Ignoring push to " + repo + " on branch " + ref}
        };
        return crow::response(200, response.dump());
    }

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
        std::string action_name;
        std::string action_command;
        nlohmann::json action_args;
        try {
            action_name = action["name"];
            action_command = action["command"];
            action_args = action["args"];
        } catch (nlohmann::json::out_of_range& e) {
            Logger::error("[/run-actions] Invalid action configuration for repo " + repo + ": " + std::string(e.what()));
            nlohmann::json response = {
                {"status", 500},
                {"error", "Invalid action configuration"}
            };
            return crow::response(500, response.dump());
        }

        Logger::info("[/run-actions] Running action '" + action_name + "'");

        std::string action_command_with_args = action_command;
        for (const auto &arg : action_args) {
            action_command_with_args += " " + arg.get<std::string>();
        }

        int ret = std::system(action_command_with_args.c_str());
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

