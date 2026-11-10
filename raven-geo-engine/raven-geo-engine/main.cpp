#include <cstdlib>
#include <iostream>
#include <memory>
#include <string>
#include <vector>
#include <spdlog/spdlog.h>
#include <spdlog/fmt/ranges.h>
#include "Common/Constants.hpp"
#include "Common/DotEnv.hpp"
#include "Network/TcpSocketReceiver.hpp"
#include "Math/GeoCalculator.hpp"
#include "Terrain/GeoTiffHeightProvider.hpp"

using Terrain::GeoTiffHeightProvider;
using std::nullopt;

namespace
{
	std::vector<std::string> readTilePaths()
	{
		std::vector<std::string> tilePaths;
		for (const char* envVarName : Constants::GeoTiff::TILE_PATH_ENV_VARS)
		{
			const char* tilePath = std::getenv(envVarName);
			if (tilePath == nullptr)
			{
				spdlog::warn(Constants::Logs::TILE_PATH_ENV_MISSING, envVarName);
				continue;
			}
			tilePaths.push_back(tilePath);
		}
		return tilePaths;
	}

	// compare the DSM height at the frame center with the telemetry's own frame center elevation (log only)
	void logDsmVsFrameCenter(const Models::TelemetryFrame& frame, const Terrain::IHeightProvider& heightProvider)
	{
		if (frame.frameCenterLatitude == nullopt || frame.frameCenterLongitude == nullopt)
		{
			return;
		}

		std::optional<double> dsmHeight = heightProvider.getHeightMeters(frame.frameCenterLatitude.value(), frame.frameCenterLongitude.value());
		if (dsmHeight == nullopt)
		{
			spdlog::warn(Constants::Logs::DSM_NO_TILE_AT_FRAME_CENTER, frame.frameCenterLatitude.value(), frame.frameCenterLongitude.value());
		}
		else if (frame.frameCenterElevation != nullopt)
		{
			double diff = frame.frameCenterElevation.value() - dsmHeight.value();
			spdlog::info(Constants::Logs::DSM_VS_FRAME_CENTER_ELEVATION, frame.frameCenterElevation.value(), dsmHeight.value(), diff);
		}
		else
		{
			spdlog::info(Constants::Logs::DSM_AT_FRAME_CENTER_NO_ELEVATION, dsmHeight.value());
		}
	}

	// log where the ray hit and, when the frame center is known, how far the hit is from it
	void logHitPoint(const Models::TelemetryFrame& frame, const Models::GroundIntersection& hitPoint, Math::GeoCalculator& calculator)
	{
		if (frame.frameCenterLatitude != nullopt && frame.frameCenterLongitude != nullopt)
		{
			double errorMeters = calculator.haversineDistanceMeters(
				hitPoint.latitude, hitPoint.longitude, frame.frameCenterLatitude.value(), frame.frameCenterLongitude.value());

			spdlog::info(Constants::Logs::CALC_GROUND_INTERSECTION_ERROR,
				hitPoint.latitude, hitPoint.longitude, hitPoint.slantRangeMeters, errorMeters);
		}
		else
		{
			spdlog::info(Constants::Logs::CALC_GROUND_INTERSECTION,
				hitPoint.latitude, hitPoint.longitude, hitPoint.slantRangeMeters);
			spdlog::warn(Constants::Logs::FRAME_CENTER_LATITUDE_AND_LONGITUDE_MISSING);
		}
	}
}

int main()
{
    spdlog::info(Constants::Logs::ENGINE_STARTING);

    // first, so every setting below (like the tiles folder) can come from the .env file
    Common::loadDotEnv(Constants::DotEnv::FILE_NAME);

    // load every terrain tile once, before any frame arrives.
    // created first so it outlives the calculator that borrows it
    GeoTiffHeightProvider heightProvider(readTilePaths());
    auto calculator = std::make_shared<Math::GeoCalculator>(heightProvider);
    Network::TcpSocketReceiver receiver(Constants::Network::DEFAULT_HOST, Constants::Network::LTS_SOCKET_PORT);

    receiver.start([&calculator, &heightProvider](const Models::TelemetryFrame& frame)
    {
		spdlog::info(Constants::Logs::START_CALC);

		// a missing ray input would silently become a wrong ray, so skip the frame
        std::vector<std::string> missingFields = calculator->findMissingRayFields(frame);
        if (!missingFields.empty())
        {
            spdlog::warn(Constants::Logs::RAY_FIELDS_MISSING, fmt::join(missingFields, ", "));
            return;
        }

        logDsmVsFrameCenter(frame, heightProvider);

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

        logHitPoint(frame, *hitPoint, *calculator);
        spdlog::info(Constants::Logs::CALCULATION_COMPLETE);
    });

    spdlog::info(Constants::Logs::RUNNING_WAITING);
    std::cin.get();

    spdlog::info(Constants::Logs::ENGINE_STOPPING);
    receiver.stop();

    return 0;
}
