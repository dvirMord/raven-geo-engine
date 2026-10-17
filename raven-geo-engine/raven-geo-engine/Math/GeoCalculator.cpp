#include "GeoCalculator.hpp"
#include <spdlog/spdlog.h>

namespace Math 
{
    Models::Ray GeoCalculator::calculateRay(const Models::TelemetryFrame& frame) 
    {
        spdlog::info(Constants::Logs::CALC_RAY, frame.gimbalAzimuthDeg);
        Models::Ray ray;
        return ray;
    }
}