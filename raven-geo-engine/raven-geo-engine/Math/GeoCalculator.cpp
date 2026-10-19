#include "GeoCalculator.hpp"
#include <spdlog/spdlog.h>
#include <numbers>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/glm.hpp>

using std::numbers::pi;

namespace Math 
{
    Models::Ray GeoCalculator::calculateRay(const Models::TelemetryFrame& frame) 
    {
        spdlog::info(Constants::Logs::CALC_RAY, frame.gimbalAzimuthDeg);
        Models::Ray ray;
        return ray;
    }

	double GeoCalculator::toRadians(const double degrees)
	{
		return degrees * (pi / 180.0);
	}

    glm::dmat3 GeoCalculator::buildGimbalRotation(double panDeg, double tiltDeg)
    {
        double pan = toRadians(panDeg);
        double tilt = toRadians(tiltDeg);

        glm::dmat3 Ry_tilt = glm::dmat3(
            std::cos(tilt), 0.0, -std::sin(tilt),
            0.0, 1.0, 0.0,
            std::sin(tilt), 0.0, std::cos(tilt)
        );

        glm::dmat3 Rz_pan = glm::dmat3(
            std::cos(pan), std::sin(pan), 0.0,
            -std::sin(pan), std::cos(pan), 0.0,
            0.0, 0.0, 1.0
        );

        return Rz_pan * Ry_tilt;
    }

    glm::dmat3 GeoCalculator::buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg)
    {
        double roll = toRadians(rollDeg);
        double pitch = toRadians(pitchDeg);
        double yaw = toRadians(yawDeg);

        glm::dmat3 Rx_roll = glm::dmat3(
            1.0, 0.0, 0.0,
            0.0, std::cos(roll), std::sin(roll),
            0.0, -std::sin(roll), std::cos(roll)
        );

        glm::dmat3 Ry_pitch = glm::dmat3(
            std::cos(pitch), 0.0, -std::sin(pitch),
            0.0, 1.0, 0.0,
            std::sin(pitch), 0.0, std::cos(pitch)
        );

        glm::dmat3 Rz_yaw = glm::dmat3(
            std::cos(yaw), std::sin(yaw), 0.0,
            -std::sin(yaw), std::cos(yaw), 0.0,
            0.0, 0.0, 1.0
        );

        return Rz_yaw * Ry_pitch * Rx_roll;
    }

    glm::dmat3 GeoCalculator::calculateCameraToWorldMatrix(
        double uavRollDeg, double uavPitchDeg, double uavYawDeg,
        double gimbalPanDeg, double gimbalTiltDeg)
    {
        glm::dmat3 rGimbal = buildGimbalRotation(gimbalPanDeg, gimbalTiltDeg);
        glm::dmat3 rBody = buildBodyRotation(uavRollDeg, uavPitchDeg, uavYawDeg);

        return rBody * rGimbal;
    }

    glm::dvec3 GeoCalculator::calculateVWorld(
        double uavRollDeg, double uavPitchDeg, double uavYawDeg,
        double gimbalPanDeg, double gimbalTiltDeg)
    {

        glm::dvec3 vCam(1.0, 0.0, 0.0);

        glm::dmat3 rTotal = calculateCameraToWorldMatrix(
            uavRollDeg, uavPitchDeg, uavYawDeg,
            gimbalPanDeg, gimbalTiltDeg
        );

        return rTotal * vCam;
    }


}