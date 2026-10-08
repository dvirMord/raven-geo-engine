#include "crow.h"
#include "GeoService.h"
#include "GeoController.h"

int main()
{
    crow::SimpleApp app;

    GeoService geoService;

    GeoController geoController(app, geoService);

    CROW_ROUTE(app, "/health")([]() {
        crow::json::wvalue res;
        res["Message"] = "Engine API is UP!";
        return crow::response(200, res);
        });

    app.port(18080).multithreaded().run();
}