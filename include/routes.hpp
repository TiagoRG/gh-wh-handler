#ifndef ROUTES_HPP
#define ROUTES_HPP

#include <crow/app.h>
#include <nlohmann/json.hpp>

class Routes {
    public:
        Routes(nlohmann::json);

    private:
        crow::SimpleApp app;
};

#endif
