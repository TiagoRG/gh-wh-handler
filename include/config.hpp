#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <nlohmann/json.hpp>

class Config {
    public:
        static void create_config(std::string config_file_path);
        static nlohmann::json get_config(std::string config_file_path);
        static void open_menu(nlohmann::json config, std::string config_file_path);

    private:
        static void open_terminal_only_menu(nlohmann::json config, std::string config_file_path);

        static nlohmann::json set_port(nlohmann::json config, int port);

        static nlohmann::json add_update_files_repo(nlohmann::json config, std::string repo, std::string branch);
        static nlohmann::json add_update_files_file(nlohmann::json config, std::string repo, std::string remote_path, std::string local_path);
        static nlohmann::json add_update_files_post_update(nlohmann::json config, std::string repo, std::string command);
        static nlohmann::json add_run_scripts_repo(nlohmann::json config, std::string repo, std::string branch);
        static nlohmann::json add_run_actions_action(nlohmann::json config, std::string repo, std::string script_path);
        static nlohmann::json add_token(nlohmann::json config, std::string repo, std::string token);

        static nlohmann::json remove_update_files_repo(nlohmann::json config, std::string repo);
        static nlohmann::json remove_update_files_file(nlohmann::json config, std::string repo, std::string remote_path);
        static nlohmann::json remove_update_files_post_update(nlohmann::json config, std::string repo, std::string command);
        static nlohmann::json remove_run_scripts_repo(nlohmann::json config, std::string repo);
        static nlohmann::json remove_run_actions_action(nlohmann::json config, std::string repo, std::string script_path);
        static nlohmann::json remove_token(nlohmann::json config, std::string repo);
};

#endif
