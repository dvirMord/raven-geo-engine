#pragma once
#include "../Models/TelemetryFrame.hpp"
#include "../Models/Ray.hpp"
#include "../Models/GroundIntersection.hpp"
#include "Vector3D.hpp"
#include "Matrix3x3.hpp"
#include <optional>
#include <string>
#include <vector>

using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

namespace Math 
{
    class IGeoCalculator 
    {
    public:
        virtual ~IGeoCalculator() = default;
        virtual Models::Ray calculateRay(const Models::TelemetryFrame& frame) = 0;

		// names of the ray inputs missing in the frame - empty when the ray can be calculated
        virtual std::vector<std::string> findMissingRayFields(const Models::TelemetryFrame& frame) = 0;
        
		//math functions
        virtual double haversineDistanceMeters(const double lat1, const double lon1, const double lat2, const double lon2) = 0;

		// calc the VWorld vector from the UAV and gimbal angles
        virtual Matrix3x3 buildGimbalRotation(double panDeg, double tiltDeg) = 0;
        virtual Matrix3x3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) = 0;
        virtual Matrix3x3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
        virtual Vector3D calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
    
		// calc the ground intersection point from the ray and the ground plane
        virtual std::optional<Models::GroundIntersection> castRayToFlatGround(const Models::Ray& ray, double groundAltitudeMeters) = 0;
    };
}