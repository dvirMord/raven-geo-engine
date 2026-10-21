#pragma once
#include "IGeoCalculator.hpp"
#include "../Common/Constants.hpp"
#include <iostream>

using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

namespace Math 
{

    class GeoCalculator : public IGeoCalculator 
    {
    public:
        GeoCalculator() = default;
        ~GeoCalculator() override = default;

		//math functions
        double toRadians(const double degrees) override;
		double toDegrees(const double radians) override;
		double haversineDistanceMeters(const double lat1, const double lon1, const double lat2, const double lon2) override;

        // clac the VWorld vector from the UAV and gimbal angles
        Matrix3x3 buildGimbalRotation(double panDeg, double tiltDeg) override;
        Matrix3x3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) override;
        Matrix3x3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) override;
        Vector3D calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) override;

        // calculate the ray from the telemetry frame
        Models::Ray calculateRay(const Models::TelemetryFrame& frame) override;

		// names of the ray inputs missing in the frame - empty when the ray can be calculated
        std::vector<std::string> findMissingRayFields(const Models::TelemetryFrame& frame) override;

		// clac the ground intersection point from the ray and the ground plane
		std::optional<Models::GroundIntersection> castRayToFlatGround(const Models::Ray& ray, double groundAltitudeMeters) override;
    };

} 
