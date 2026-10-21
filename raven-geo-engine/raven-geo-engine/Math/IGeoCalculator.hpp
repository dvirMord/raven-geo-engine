#pragma once
#include "../Models/TelemetryFrame.hpp"
#include "../Models/Ray.hpp"
#include "../Models/GroundIntersection.hpp"
#include "Vector3D.hpp"
#include "Matrix3x3.hpp"
#include <optional>

using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

namespace Math 
{
    class IGeoCalculator 
    {
    public:
        virtual ~IGeoCalculator() = default;
        virtual Models::Ray calculateRay(const Models::TelemetryFrame& frame) = 0;
        
		//math functions
        virtual double toRadians(const double degrees) = 0;
		virtual double toDegrees(const double radians) = 0;
        virtual double haversineDistanceMeters(const double lat1, const double lon1, const double lat2, const double lon2) = 0;

		// clac the VWorld vector from the UAV and gimbal angles
        virtual Matrix3x3 buildGimbalRotation(double panDeg, double tiltDeg) = 0;
        virtual Matrix3x3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) = 0;
        virtual Matrix3x3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
        virtual Vector3D calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
    
		// clac the ground intersection point from the ray and the ground plane
        virtual std::optional<Models::GroundIntersection> castRayToFlatGround(const Models::Ray& ray, double groundAltitudeMeters) = 0;
    };
}