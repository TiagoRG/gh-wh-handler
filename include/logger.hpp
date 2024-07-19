#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <string>

class Logger {
    public:
        static void init(std::string log_file_path);
        static void info(std::string message);
        static void warn(std::string message);
        static void error(std::string message);
        static void fatal(std::string message);
        static void success(std::string message);
        static void code(std::string message);
    private:
        static void log(std::string message, std::string level);
        static std::ofstream log_file;
};

#endif
