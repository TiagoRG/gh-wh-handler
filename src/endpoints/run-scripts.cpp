#include "endpoints/run-scripts.hpp"

crow::response run_scripts(const nlohmann::json &run_scripts, const nlohmann::json &tokens, const crow::request &req) {
    // TODO: Implement run_scripts
    nlohmann::json response = {
        {"status", 501},
        {"error", "Not Implemented"}
    };
    return crow::response(501, response.dump());
}

