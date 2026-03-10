#ifndef DIALOG_MENU_HPP
#define DIALOG_MENU_HPP

#include <nlohmann/json.hpp>

class DialogMenu {
    public:
        static void open_menu(nlohmann::json config, std::string config_file_path, std::string default_editor);

    private:
        static void open_main_menu();

        static nlohmann::json s_config;
        static std::string s_config_path;
        static std::string s_editor;
};

#endif
