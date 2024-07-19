#include "routes.hpp"
#include "endpoints/update-files.hpp"
#include "endpoints/run-scripts.hpp"
#include <crow/app.h>

#include "logger.hpp"

Routes::Routes(nlohmann::json config) {
    Logger::info("Partitioning configuration");
    const nlohmann::json config_update_files = config["update-files"];
    const nlohmann::json config_run_scripts = config["run-scripts"];
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

    Logger::info("Registering route \"/run-scripts\"");
    CROW_ROUTE(this->app, "/run-scripts")
        .methods("POST"_method)
        .name("Run Scripts")
        ([&config_run_scripts, &config_tokens](const crow::request &req) {
            try {
                return run_scripts(config_run_scripts, config_tokens, req);
            } catch (const std::exception &e) {
                Logger::error("Unknown error in run_scripts: " + std::string(e.what()));
                nlohmann::json response = {
                    {"status", 500},
                    {"error", "Internal server error"}
                };
                return crow::response(500, response.dump());
            }
        });

    Logger::info("Starting server");
    this->app.port(config["port"].get<int>()).multithreaded().run();
    Logger::info("Server stopped");
}
