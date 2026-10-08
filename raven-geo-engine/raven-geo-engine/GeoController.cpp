#include "GeoController.hpp"
#include "Constants.hpp"

GeoController::GeoController(crow::SimpleApp& app, GeoService& geoService)
    : _geoService(geoService)
{
    setupRoutes(app);
}

void GeoController::setupRoutes(crow::SimpleApp& app) {
    CROW_ROUTE(app, Constants::Routes::CALCULATE).methods(crow::HTTPMethod::POST)
        ([this](const crow::request& req) {
            auto body = crow::json::load(req.body);
            if (!body) {
                return crow::response(Constants::StatusCodes::BAD_REQUEST, Constants::Errors::INVALID_JSON);
            }

            if (!body.has(Constants::JsonKeys::LATITUDE) ||
                !body.has(Constants::JsonKeys::LONGITUDE) ||
                !body.has(Constants::JsonKeys::ALTITUDE)) {
                return crow::response(Constants::StatusCodes::BAD_REQUEST, Constants::Errors::MISSING_FIELDS);
            }

            double lat = body[Constants::JsonKeys::LATITUDE].d();
            double lon = body[Constants::JsonKeys::LONGITUDE].d();
            double alt = body[Constants::JsonKeys::ALTITUDE].d();

            std::string result = this->_geoService.calculateIntersection(lat, lon, alt);

            crow::json::wvalue res;
            res[Constants::JsonKeys::STATUS] = Constants::Messages::SUCCESS;
            res[Constants::JsonKeys::DATA] = result;

            return crow::response(Constants::StatusCodes::OK, res);
        });
}
