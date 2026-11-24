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
#include "RayMarching/BlockSkipRayMarcher.hpp"

using RayMarching::BlockSkipRayMarcher;
using RayMarching::IRayMarching;
using Terrain::GeoTiffHeightProvider;
using Models::RayHitResult;
using Models::RayHitStatus;
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


	// log where the ray hit and, when the frame center is known, how far the hit is from it
	void logHitPoint(const Models::TelemetryFrame& frame, const Models::GroundIntersection& hitPoint, Math::GeoCalculator& calculator)
	{
		if (frame.frameCenterLatitude != nullopt && frame.frameCenterLongitude != nullopt)
		{
			double errorMeters = calculator.haversineDistanceMeters(
				hitPoint.latitude, hitPoint.longitude, frame.frameCenterLatitude.value(), frame.frameCenterLongitude.value());

			spdlog::info(Constants::Logs::CALC_GROUND_INTERSECTION_ERROR,
				hitPoint.latitude, hitPoint.longitude, hitPoint.altitudeMeters, hitPoint.slantRangeMeters, errorMeters);
		}
		else
		{
			spdlog::info(Constants::Logs::CALC_GROUND_INTERSECTION,
				hitPoint.latitude, hitPoint.longitude, hitPoint.altitudeMeters, hitPoint.slantRangeMeters);
			spdlog::warn(Constants::Logs::FRAME_CENTER_LATITUDE_AND_LONGITUDE_MISSING);
		}
	}
}

int main()
{
    spdlog::info(Constants::Logs::ENGINE_STARTING);

    // first, so every setting can come from the .env file
    Common::loadDotEnv(Constants::DotEnv::FILE_NAME);

    // load every terrain tile once, before any frame arrives.
    // created first so it outlives the calculator that borrows it
    GeoTiffHeightProvider heightProvider(readTilePaths());

	BlockSkipRayMarcher rayMarcher(heightProvider);

    auto calculator = std::make_shared<Math::GeoCalculator>();
    Network::TcpSocketReceiver receiver(Constants::Network::DEFAULT_HOST, Constants::Network::LTS_SOCKET_PORT);

    // counts the frames so each block of log lines can be told apart
    std::uint64_t frameNumber = 0;

    receiver.start([&calculator, &rayMarcher, &frameNumber](const Models::TelemetryFrame& frame)
    {
		spdlog::info(Constants::Logs::START_CALC, ++frameNumber);

		// a missing ray input would silently become a wrong ray, so skip the frame
        std::vector<std::string> missingFields = calculator->findMissingRayFields(frame);
        if (!missingFields.empty())
        {
            spdlog::warn(Constants::Logs::RAY_FIELDS_MISSING, fmt::join(missingFields, ", "));
            return;
        }

		// Calculate the ray from the telemetry frame
        Models::Ray ray = calculator->calculateRay(frame);


		// Calculate the ground intersection point from the ray and the ground
		
		RayHitResult hitPoint = rayMarcher.getFirstHit(ray);
		switch (hitPoint.hitStatus)
		{
		case RayHitStatus::Hit:
			logHitPoint(frame, hitPoint.intersectionPoint.value(), *calculator);
			break;
		case RayHitStatus::Miss:
			spdlog::info(Constants::Logs::RAY_MARCH_MISS, Constants::RayMarching::MAX_RAY_DISTANCE_METERS);
			break;
		case RayHitStatus::NoTerrainData:
			spdlog::warn(Constants::Logs::RAY_MARCH_NO_TERRAIN_DATA);
			break;
		}

        
    });

    spdlog::info(Constants::Logs::RUNNING_WAITING);
    std::cin.get();

    spdlog::info(Constants::Logs::ENGINE_STOPPING);
    receiver.stop();

    return 0;
}
