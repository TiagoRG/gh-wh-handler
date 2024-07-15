#include <crow/http_request.h>
#include <crow/http_response.h>
#include <nlohmann/json.hpp>

crow::response update_files(const nlohmann::json &, const nlohmann::json &, const crow::request &);
