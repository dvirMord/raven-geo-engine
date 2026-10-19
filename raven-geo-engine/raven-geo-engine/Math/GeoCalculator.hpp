#pragma once
#include "IGeoCalculator.hpp"
#include "../Common/Constants.hpp"
#include <iostream>
#include <glm/glm.hpp>

namespace Math 
{

    class GeoCalculator : public IGeoCalculator 
    {
    public:
        GeoCalculator() = default;
        ~GeoCalculator() override = default;

        Models::Ray calculateRay(const Models::TelemetryFrame& frame) override;

        double toRadians(const double degrees) override;
        glm::dmat3 buildGimbalRotation(double panDeg, double tiltDeg) override;
        glm::dmat3 buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg) override;
        glm::dmat3 calculateCameraToWorldMatrix(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) override;
        glm::dvec3 calculateVWorld(double uavRollDeg, double uavPitchDeg, double uavYawDeg, double gimbalPanDeg, double gimbalTiltDeg) override;
    };

} 
