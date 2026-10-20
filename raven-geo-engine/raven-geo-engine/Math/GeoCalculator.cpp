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
		return degrees * (pi / Constants::Math::SEMICIRCLE_DEGREES);
	}

    glm::dmat3 GeoCalculator::buildGimbalRotation(double panDeg, double tiltDeg)
    {
        double pan = toRadians(panDeg);
        double tilt = toRadians(tiltDeg);

        double cosTilt = std::cos(tilt);
        double sinTilt = std::sin(tilt);
        double cosPan = std::cos(pan);
        double sinPan = std::sin(pan);

        glm::dmat3 Ry_tilt = glm::dmat3(
            cosTilt, 0.0, -sinTilt,
            0.0, 1.0, 0.0,
            sinTilt, 0.0, cosTilt
        );

        glm::dmat3 Rz_pan = glm::dmat3(
            cosPan, sinPan, 0.0,
            -sinPan, cosPan, 0.0,
            0.0, 0.0, 1.0
        );

        return Rz_pan * Ry_tilt;
    }

    glm::dmat3 GeoCalculator::buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg)
    {
        double roll = toRadians(rollDeg);
        double pitch = toRadians(pitchDeg);
        double yaw = toRadians(yawDeg);

        double cosRoll = std::cos(roll);
        double sinRoll = std::sin(roll);
        double cosPitch = std::cos(pitch);
        double sinPitch = std::sin(pitch);
        double cosYaw = std::cos(yaw);
        double sinYaw = std::sin(yaw);

        glm::dmat3 Rx_roll = glm::dmat3(
            1.0, 0.0, 0.0,
            0.0, cosRoll, sinRoll,
            0.0, -sinRoll, cosRoll
        );

        glm::dmat3 Ry_pitch = glm::dmat3(
            cosPitch, 0.0, -sinPitch,
            0.0, 1.0, 0.0,
            sinPitch, 0.0, cosPitch
        );

        glm::dmat3 Rz_yaw = glm::dmat3(
            cosYaw, sinYaw, 0.0,
            -sinYaw, cosYaw, 0.0,
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