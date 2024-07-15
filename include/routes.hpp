#include <crow/app.h>
#include <nlohmann/json.hpp>

class Routes {
    public:
        Routes(nlohmann::json);
        ~Routes();

    private:
        crow::SimpleApp app;
};
