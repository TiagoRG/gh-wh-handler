#include "logger.hpp"
#include <filesystem>
#include <iostream>
#include <fstream>
#include <ctime>
#include <unistd.h>
#include <sys/ioctl.h>

#define COLORS_RESET "\033[0m"
#define COLORS_BOLD "\033[1m"
#define COLORS_DIM "\033[2m"
#define COLORS_UNDERLINED "\033[4m"
#define COLORS_BLINK "\033[5m"
#define COLORS_REVERSE "\033[7m"
#define COLORS_HIDDEN "\033[8m"

#define COLORS_FG_BLACK "\033[30m"
#define COLORS_FG_RED "\033[31m"
#define COLORS_FG_GREEN "\033[32m"
#define COLORS_FG_YELLOW "\033[33m"
#define COLORS_FG_BLUE "\033[34m"
#define COLORS_FG_MAGENTA "\033[35m"
#define COLORS_FG_CYAN "\033[36m"
#define COLORS_FG_WHITE "\033[37m"

#define COLORS_BG_BLACK "\033[40m"
#define COLORS_BG_RED "\033[41m"
#define COLORS_BG_GREEN "\033[42m"
#define COLORS_BG_YELLOW "\033[43m"
#define COLORS_BG_BLUE "\033[44m"
#define COLORS_BG_MAGENTA "\033[45m"
#define COLORS_BG_CYAN "\033[46m"
#define COLORS_BG_WHITE "\033[47m"

std::ofstream Logger::log_file;

void Logger::init(std::string logs_dir) {
    std::cout << "Initializing logger" << std::endl;
    std::cout << "Logs directory: " << logs_dir << std::endl;
    // check if logs_dir exists
    if (!std::filesystem::exists(logs_dir)) {
        try {
            std::filesystem::create_directories(logs_dir);
        } catch (std::exception &e) {
            std::cerr << "Error creating logs directory: " << e.what() << std::endl;
            std::exit(1);
        }
    }
    // check if logs_dir ends with a slash
    if (logs_dir.back() != '/') {
        logs_dir += "/";
    }
    std::time_t now = std::time(nullptr);
    std::tm *now_tm = std::localtime(&now);
    char time_buffer[80];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", now_tm);

    std::string log_file_path = logs_dir + "gh-wh-handler_" + time_buffer + ".log";

    std::cout << "Log file: " << log_file_path << std::endl;
    Logger::log_file.open(log_file_path, std::ios::app);
    if (!Logger::log_file.is_open()) {
        std::cerr << "Error opening log file" << std::endl;
    }
    Logger::success("Logger initialized");
}

void Logger::info(std::string message) {
    Logger::log(message, "INFO    ");
}

void Logger::warn(std::string message) {
    Logger::log(message, "WARN    ");
}

void Logger::error(std::string message) {
    Logger::log(message, "ERROR   ");
}

void Logger::fatal(std::string message) {
    Logger::log(message, "FATAL   ");
}

void Logger::success(std::string message) {
    Logger::log(message, "SUCCESS ");
}

void Logger::code(std::string message) {
    Logger::log(message, "CODE");
}

void Logger::debug(std::string message) {
    Logger::log(message, "DEBUG   ");
}

void Logger::log(std::string message, std::string level) {
    std::string formatted_message = "";
    if (isatty(fileno(stdout))) {
        if (level == "INFO    ") {
            formatted_message += COLORS_FG_GREEN;
        } else if (level == "WARN    ") {
            formatted_message += COLORS_FG_YELLOW;
        } else if (level == "ERROR   ") {
            formatted_message += COLORS_FG_RED;
        } else if (level == "FATAL   ") {
            formatted_message += COLORS_FG_RED;
            formatted_message += COLORS_BOLD;
        } else if (level == "SUCCESS ") {
            formatted_message += COLORS_FG_GREEN;
            formatted_message += COLORS_BOLD;
        } else if (level == "CODE") {
            formatted_message += COLORS_FG_WHITE;
            formatted_message += COLORS_DIM;
        } else if (level == "DEBUG   ") {
            formatted_message += COLORS_FG_BLUE;
            formatted_message += COLORS_UNDERLINED;
            formatted_message += COLORS_DIM;
            formatted_message += COLORS_BOLD;
        }
    }

    std::time_t now = std::time(nullptr);
    std::tm *now_tm = std::localtime(&now);
    char time_buffer[80];
    std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d %H:%M:%S", now_tm);
    formatted_message += "(" + std::string(time_buffer) + ") ";

    if (level == "CODE") {
#if defined(__aarch64__)
        formatted_message += "\n" + message + "\n";
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_width = w.ws_col > 250 ? 250 : w.ws_col - 1;
        formatted_message += "\n" + std::string(term_width, '=') + "\n" + message + "\n" + std::string(term_width, '=');
#endif
    } else {
        formatted_message += "[" + level + "] " + message;
    }
    if (isatty(fileno(stdout))) {
        formatted_message += COLORS_RESET;
    }
    std::cout << formatted_message << std::endl;
    if (level == "CODE") {
#if defined(__aarch64__)
        Logger::log_file << std::endl << message << std::endl;
#else
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
        int term_width = w.ws_col > 250 ? 250 : w.ws_col - 1;
        Logger::log_file << std::string(term_width, '=') << std::endl << message << std::endl << std::string(term_width, '=') << std::endl;
#endif
    } else {
        Logger::log_file << "[" << level << "] " << message << std::endl;
    }

    Logger::log_file.flush();

    if (level == "FATAL   ") {
        std::exit(1);
    }
}
