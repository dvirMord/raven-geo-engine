#pragma once
#include <string>
#include "Constants.hpp"

class GeoService {
public:
    GeoService() = default;

    std::string calculateIntersection(double lat, double lon, double alt) {
        return std::string(Constants::GeoCalculation::LAT_PREFIX) + std::to_string(lat) +
            Constants::GeoCalculation::LON_PREFIX + std::to_string(lon);
    }
};