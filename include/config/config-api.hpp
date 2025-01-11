#ifndef CONFIG_API_HPP
#define CONFIG_API_HPP

#include <nlohmann/json.hpp>

class ConfigApi {
    public:
        static bool set_port(nlohmann::json config, int port);
        static bool add_token(nlohmann::json config, std::string repo, std::string token);
        static bool remove_token(nlohmann::json config, std::string repo);
};

#endif

