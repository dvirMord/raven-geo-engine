#pragma once

#include <string>

class GeoService {
public:
    GeoService() = default;
    ~GeoService() = default;

    std::string calculateIntersection(double lat, double lon, double alt);
};