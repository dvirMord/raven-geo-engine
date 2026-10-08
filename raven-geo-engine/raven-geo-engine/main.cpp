#include "crow.h"
#include "GeoService.hpp"
#include "GeoController.hpp"
#include "Constants.hpp"

void registerHealthRoute(crow::SimpleApp& app) {
    CROW_ROUTE(app, Constants::Routes::HEALTH)([]() {
        crow::json::wvalue res;
        res[Constants::JsonKeys::MESSAGE] = Constants::Messages::ENGINE_UP;
        return crow::response(Constants::StatusCodes::OK, res);
    });
}

void runServer(crow::SimpleApp& app) {
    app.port(Constants::Server::DEFAULT_PORT).multithreaded().run();
}

int main() {
    crow::SimpleApp app;

    GeoService geoService;
    GeoController geoController(app, geoService);

    registerHealthRoute(app);
    runServer(app);

    return 0;
}