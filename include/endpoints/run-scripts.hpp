#ifndef RUN_SCRIPTS_HPP
#define RUN_SCRIPTS_HPP

#include <crow/http_request.h>
#include <crow/http_response.h>
#include <nlohmann/json.hpp>

crow::response run_scripts(const nlohmann::json &, const nlohmann::json &,const crow::request &);

#endif
