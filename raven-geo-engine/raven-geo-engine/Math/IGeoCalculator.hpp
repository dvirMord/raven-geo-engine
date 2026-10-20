#pragma once
#include "../Models/TelemetryFrame.hpp"
#include "../Models/Ray.hpp"
#include "Vector3D.hpp"
#include "Matrix3x3.hpp"

using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

namespace Math 
{
    class IGeoCalculator 
    {
    public:
        virtual ~IGeoCalculator() = default;
        virtual Models::Ray calculateRay(const Models::TelemetryFrame& frame) = 0;
        virtual double toRadians(const double degrees) = 0;
        virtual Matrix3x3 buildGimbalRotation(double panDeg, double tiltDeg) = 0;
        virtual Matrix3x3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) = 0;
        virtual Matrix3x3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
        virtual Vector3D calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) = 0;
    };
}