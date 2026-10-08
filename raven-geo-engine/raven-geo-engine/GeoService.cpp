#include "GeoService.hpp"
#include "Constants.hpp"

std::string GeoService::calculateIntersection(double lat, double lon, double alt) {
    return std::string(Constants::GeoCalculation::LAT_PREFIX) + std::to_string(lat) +
           Constants::GeoCalculation::LON_PREFIX + std::to_string(lon) +
           Constants::GeoCalculation::ALT_PREFIX + std::to_string(alt);
}
