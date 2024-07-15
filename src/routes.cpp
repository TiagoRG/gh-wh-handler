#include "routes.hpp"
#include "endpoints/update-files.hpp"
#include "endpoints/run-scripts.hpp"
#include <crow/app.h>

Routes::Routes(nlohmann::json config) {
    const nlohmann::json config_update_files = config["update-files"];
    const nlohmann::json config_run_scripts = config["run-scripts"];
    const nlohmann::json config_tokens = config["tokens"];

    CROW_ROUTE(this->app, "/update-files")
        .methods("POST"_method)
        .name("Update Files")
        ([&config_update_files, &config_tokens](const crow::request &req) {
            try {
                return update_files(config_update_files, config_tokens, req);
            } catch (const std::exception &e) {
                std::cerr << "Unknown error in update_files: " << e.what() << std::endl;
                nlohmann::json response = {
                    {"status", 500},
                    {"error", "Internal server error"}
                };
                return crow::response(500, response.dump());
            }
        });

    CROW_ROUTE(this->app, "/run-scripts")
        .methods("POST"_method)
        .name("Run Scripts")
        ([&config_run_scripts, &config_tokens](const crow::request &req) {
            try {
                return run_scripts(config_run_scripts, config_tokens, req);
            } catch (const std::exception &e) {
                std::cerr << "Unknown error in run_scripts: " << e.what() << std::endl;
                nlohmann::json response = {
                    {"status", 500},
                    {"error", "Internal server error"}
                };
                return crow::response(500, response.dump());
            }
        });

    this->app.port(config["port"].get<int>()).multithreaded().run();
}
