#ifndef TERMINAL_MENU_HPP
#define TERMINAL_MENU_HPP

#include <nlohmann/json.hpp>

class TerminalMenu {
    public:
        static void open_menu(nlohmann::json config, std::string config_file_path);

    private:
        static void general_menu(nlohmann::json config, std::string config_file_path);
        static void update_files_menu(nlohmann::json config, std::string config_file_path);
        static void run_scripts_menu(nlohmann::json config, std::string config_file_path);
};

#endif
