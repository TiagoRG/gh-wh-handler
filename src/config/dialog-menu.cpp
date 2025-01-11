#include <dialog.h>
#include "config/dialog-menu.hpp"
#include "logger.hpp"

void DialogMenu::open_menu(nlohmann::json config, std::string config_file_path) {
    Logger::info("Opening dialog menu");
    init_dialog(stdin, stdout);
    dialog_msgbox(
        "GitHub Webhook Handler - Config Menu",
        "\nWelcome to the config menu!\n\nThis is a work in progress, please use the terminal menu for now.",
        10, 50, 1);
    Logger::warn("Dialog menu is a work in progress, please use the terminal menu for now.");
    end_dialog();
    Logger::info("Dialog menu closed");
}
