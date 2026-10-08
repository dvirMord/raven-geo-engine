#pragma once

#include "crow.h"
#include "GeoService.hpp"

class GeoController {
private:
    GeoService& _geoService;

    void setupRoutes(crow::SimpleApp& app);

public:
    GeoController(crow::SimpleApp& app, GeoService& geoService);
    ~GeoController() = default;
};