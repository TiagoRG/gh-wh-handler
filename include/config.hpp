#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <nlohmann/json.hpp>

class Config {
    public:
        static void create_config(std::string config_file_path);
        static nlohmann::json get_config(std::string config_file_path);
        static void open_config_menu();

    private:
        static void set_port(int port);

        static void add_update_files_repo(std::string repo, std::string branch);
        static void add_update_files_file(std::string repo, std::string remote_path, std::string local_path);
        static void add_update_files_post_update(std::string repo, std::string command);
        static void add_run_scripts_repo(std::string repo, std::string branch);
        static void add_run_scripts_script(std::string repo, std::string script_path);
        static void add_token(std::string repo, std::string token);

        static void remove_update_files_repo(std::string repo);
        static void remove_update_files_file(std::string repo, std::string remote_path);
        static void remove_update_files_post_update(std::string repo, std::string command);
        static void remove_run_scripts_repo(std::string repo);
        static void remove_run_scripts_script(std::string repo, std::string script_path);
        static void remove_token(std::string repo, std::string token);
};

#endif
