#include <iostream>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ranges.h>
#include "Common/Constants.hpp"
#include "Network/TcpSocketReceiver.hpp"
#include "Math/GeoCalculator.hpp"

int main() 
{
    spdlog::info(Constants::Logs::ENGINE_STARTING);

    auto calculator = std::make_shared<Math::GeoCalculator>();
    Network::TcpSocketReceiver receiver(Constants::Network::DEFAULT_HOST, Constants::Network::LTS_SOCKET_PORT);

    // Start background receiver with a simple callback
    receiver.start([&calculator](const Models::TelemetryFrame& frame) 
    {
		spdlog::info(Constants::Logs::START_CALC);

		// a missing ray input would silently become a wrong ray, so skip the frame
        std::vector<std::string> missingFields = calculator->findMissingRayFields(frame);
        if (!missingFields.empty())
        {
            spdlog::warn(Constants::Logs::RAY_FIELDS_MISSING, fmt::join(missingFields, ", "));
            return;
        }

		// Calculate the ray from the telemetry frame
        Models::Ray ray = calculator->calculateRay(frame);

		//if we dont get any frame center elevation, we will log a warning
        if(frame.frameCenterElevation == std::nullopt)
        {
            spdlog::warn(Constants::Logs::FRAME_CENTER_ELEVATION_MISSING);
            return;
        }

		// Calculate the ground intersection point from the ray and the ground
		std::optional<Models::GroundIntersection> hitPoint =
            calculator->castRayToFlatGround(ray, frame.frameCenterElevation.value());

        if (hitPoint == std::nullopt)
        {
            return;
        }

		// Log the calculated ground intersection point and the error in meters
        if(frame.frameCenterLatitude != std::nullopt && frame.frameCenterLongitude != std::nullopt)
        {
            double errorMeters = calculator->haversineDistanceMeters(
                hitPoint->latitude, hitPoint->longitude,frame.frameCenterLatitude.value(), frame.frameCenterLongitude.value());
            
            spdlog::info(Constants::Logs::CALC_GROUND_INTERSECTION_ERROR,
                hitPoint->latitude, hitPoint->longitude, hitPoint->slantRangeMeters, errorMeters);
        }
        else
        {
			// still log the hit point, only the error can't be calculated
            spdlog::info(Constants::Logs::CALC_GROUND_INTERSECTION,
                hitPoint->latitude, hitPoint->longitude, hitPoint->slantRangeMeters);
			spdlog::warn(Constants::Logs::FRAME_CENTER_LATITUDE_AND_LONGITUDE_MISSING);
        }
        spdlog::info(Constants::Logs::CALCULATION_COMPLETE);
    });

    spdlog::info(Constants::Logs::RUNNING_WAITING);
    std::cin.get();

    spdlog::info(Constants::Logs::ENGINE_STOPPING); 
    receiver.stop();

    return 0;
}