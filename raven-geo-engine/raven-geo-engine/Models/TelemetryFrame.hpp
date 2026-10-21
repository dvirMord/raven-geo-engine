#pragma once
#include <optional>

namespace Models 
{
    struct TelemetryFrame 
    {
        double sensorLatitude = 0.0;
        double sensorLongitude = 0.0;
        double sensorAltitudeMeters = 0.0;
        double platformHeadingDeg = 0.0;
        double platformPitchDeg = 0.0;
        double platformRollDeg = 0.0;
        double gimbalAzimuthDeg = 0.0;
        double gimbalElevationDeg = 0.0;
        double gimbalRollDeg = 0.0;
		std::optional<double> frameCenterLatitude = std::nullopt;
		std::optional<double> frameCenterLongitude = std::nullopt;
		std::optional<double> frameCenterElevation = std::nullopt;
    };
} 