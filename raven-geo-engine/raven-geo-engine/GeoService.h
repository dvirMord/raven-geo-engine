#pragma once
#include <string>
using std::string;

class GeoService {
public:
    GeoService() = default;

    string calculateIntersection(double lat, double lon, double alt) {
        return "Calculated intersection for Lat: " + std::to_string(lat) +
            ", Lon: " + std::to_string(lon);
    }
};