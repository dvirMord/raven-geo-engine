#pragma once
#include "../Models/TelemetryFrame.hpp"
#include "../Models/Ray.hpp"
#include <glm/glm.hpp>

namespace Math 
{
    class IGeoCalculator 
    {
    public:
        virtual ~IGeoCalculator() = default;
        virtual Models::Ray calculateRay(const Models::TelemetryFrame& frame) = 0;
        virtual double toRadians(const double degrees) = 0;
        virtual glm::dmat3 buildGimbalRotation(double panDeg, double tiltDeg) = 0;
        virtual glm::dmat3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) = 0;
        virtual glm::dmat3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
        virtual glm::dvec3 calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
    };
}