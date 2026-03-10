#include <dialog.h>
#include "config/dialog-menu.hpp"
#include "logger.hpp"

nlohmann::json DialogMenu::s_config;
std::string DialogMenu::s_config_path;
std::string DialogMenu::s_editor;

void DialogMenu::open_menu(nlohmann::json config, std::string config_file_path, std::string default_editor) {
    Logger::info("Opening dialog menu");

    init_dialog(stdin, stdout);

    // Define the menu items: { "Tag", "Description" }
    const char *items[] = {
        "1", "Continue with visual menu",
        "2", "Open default editor"
    };
    int item_count = 2;

    // dialog_menu returns 0 on success (Enter), 1 on Cancel/ESC
    int result = dialog_menu(
        "GitHub Webhook Handler - Config Menu", // Title
        "Select an option to proceed:",         // Prompt
        12, 50,                                 // Height, Width
        item_count,                             // Menu height inside box
        item_count,                             // Number of items
        const_cast<char**>(items)               // Items array
    );

    if (result == 0) {
        std::string choice = dialog_vars.input_result;

        if (choice == "1") {
            Logger::info("User chose: Visual Menu");

            s_config = config;
            s_config_path = config_file_path;
            s_editor = default_editor;

            open_main_menu();
        } else if (choice == "2") {
            Logger::info("User chose: Default Editor");
            // Logic to launch default_editor with config_file_path
            std::string command = default_editor + " " + config_file_path;
            system(command.c_str());
        }
    } else {
        Logger::info("User cancelled the menu");
    }

    end_dialog();
    Logger::info("Dialog menu closed");
}

void DialogMenu::open_main_menu() {
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
