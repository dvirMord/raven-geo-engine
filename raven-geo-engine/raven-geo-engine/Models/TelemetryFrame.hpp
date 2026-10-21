#pragma once
#include <optional>

namespace Models 
{
    struct TelemetryFrame 
    {
        // ray inputs - empty when missing in the telemetry, the frame is then skipped
        std::optional<double> sensorLatitude = std::nullopt;
        std::optional<double> sensorLongitude = std::nullopt;
        std::optional<double> sensorAltitudeMeters = std::nullopt;
        std::optional<double> platformHeadingDeg = std::nullopt;
        std::optional<double> platformPitchDeg = std::nullopt;
        std::optional<double> platformRollDeg = std::nullopt;
        std::optional<double> gimbalAzimuthDeg = std::nullopt;
        std::optional<double> gimbalElevationDeg = std::nullopt;
		std::optional<double> frameCenterLatitude = std::nullopt;
		std::optional<double> frameCenterLongitude = std::nullopt;
		std::optional<double> frameCenterElevation = std::nullopt;
    };
} 