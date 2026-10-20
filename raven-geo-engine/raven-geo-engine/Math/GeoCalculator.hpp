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

        Models::Ray calculateRay(const Models::TelemetryFrame& frame) override;

        double toRadians(const double degrees) override;
        Matrix3x3 buildGimbalRotation(double panDeg, double tiltDeg) override;
        Matrix3x3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) override;
        Matrix3x3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) override;
        Vector3D calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) override;
    };

} 
