#include "routes.hpp"
#include "endpoints/update-files.hpp"
#include "endpoints/run-actions.hpp"
#include <crow/app.h>

#include "logger.hpp"

Routes::Routes(nlohmann::json config) {
    Logger::info("Partitioning configuration");
    const nlohmann::json config_update_files = config["update-files"];
    const nlohmann::json config_run_actions = config["run-actions"];
    const nlohmann::json config_tokens = config["tokens"];

    Logger::info("Registering route \"/\"");
    CROW_ROUTE(this->app, "/")
        .methods("POST"_method)
        .name("Ping")
        ([]() {
            nlohmann::json response = {
                {"status", 200},
                {"message", "Pong"}
            };
            return crow::response(200, response.dump());
        });

    if (!config_update_files.is_null()) {
        Logger::info("Registering route \"/update-files\"");
        CROW_ROUTE(this->app, "/update-files")
            .methods("POST"_method)
            .name("Update Files")
            ([&config_update_files, &config_tokens](const crow::request &req) {
                try {
                    return update_files(config_update_files, config_tokens, req);
                } catch (const std::exception &e) {
                    Logger::error("Unknown error in update_files: " + std::string(e.what()));
                    nlohmann::json response = {
                        {"status", 500},
                        {"error", "Internal server error"}
                    };
                    return crow::response(500, response.dump());
                }
            });
    }

    if (!config_run_actions.is_null()) {
        Logger::info("Registering route \"/run-actions\"");
        CROW_ROUTE(this->app, "/run-actions")
            .methods("POST"_method)
            .name("Run Actions")
            ([&config_run_actions, &config_tokens](const crow::request &req) {
                try {
                    return run_actions(config_run_actions, config_tokens, req);
                } catch (const std::exception &e) {
                    Logger::error("Unknown error in run_actions: " + std::string(e.what()));
                    nlohmann::json response = {
                        {"status", 500},
                        {"error", "Internal server error"}
                    };
                    return crow::response(500, response.dump());
                }
            });
    }

    Logger::info("Starting server");
    try {
        this->app.port(config["port"].get<int>()).multithreaded().run();
    } catch (const std::exception &e) {
        Logger::fatal("Error starting server: " + std::string(e.what()));
    }
    Logger::info("Server stopped");
}
