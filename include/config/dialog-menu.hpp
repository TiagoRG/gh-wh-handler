#ifndef DIALOG_MENU_HPP
#define DIALOG_MENU_HPP

#include <nlohmann/json.hpp>

class DialogMenu {
    public:
        static void open_menu(nlohmann::json config, std::string config_file_path);
};

#endif
