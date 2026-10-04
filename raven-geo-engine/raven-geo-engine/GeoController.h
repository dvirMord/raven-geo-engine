#pragma once
#include "crow.h"
#include "GeoService.h"

class GeoController {
private:
    GeoService& _geoService;

public:
    GeoController(crow::SimpleApp& app, GeoService& geoService)
        : _geoService(geoService)
    {
        setupRoutes(app);
    }

private:
    void setupRoutes(crow::SimpleApp& app) {
        CROW_ROUTE(app, "/api/v1/calculate").methods(crow::HTTPMethod::POST)
            ([this](const crow::request& req) {

            auto body = crow::json::load(req.body);
            if (!body) {
                return crow::response(400, "Invalid JSON");
            }

            double lat = body["latitude"].d();
            double lon = body["longitude"].d();
            double alt = body["altitude"].d();

            std::string result = this->_geoService.calculateIntersection(lat, lon, alt);

            crow::json::wvalue res;
            res["status"] = "success";
            res["data"] = result;

            return crow::response(200, res);
                });
    }
};