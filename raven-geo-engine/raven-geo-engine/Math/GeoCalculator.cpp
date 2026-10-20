#include "GeoCalculator.hpp"
#include <spdlog/spdlog.h>
#include <numbers>

using std::numbers::pi;
using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

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

    Matrix3x3 GeoCalculator::buildGimbalRotation(double panDeg, double tiltDeg)
    {
        double pan = toRadians(panDeg);
        double tilt = toRadians(tiltDeg);

        Matrix3x3 Rz_pan = Matrix3x3::rotationZ(pan);
        Matrix3x3 Ry_tilt = Matrix3x3::rotationY(tilt);

        return Rz_pan * Ry_tilt;
    }

    Matrix3x3 GeoCalculator::buildBodyRotation(double rollDeg, double pitchDeg, double yawDeg)
    {
        double roll = toRadians(rollDeg);
        double pitch = toRadians(pitchDeg);
        double yaw = toRadians(yawDeg);

        Matrix3x3 Rx_roll = Matrix3x3::rotationX(roll);
        Matrix3x3 Ry_pitch = Matrix3x3::rotationY(pitch);
        Matrix3x3 Rz_yaw = Matrix3x3::rotationZ(yaw);

        return Rz_yaw * Ry_pitch * Rx_roll;
    }

    Matrix3x3 GeoCalculator::calculateCameraToWorldMatrix(
        double uavRollDeg, double uavPitchDeg, double uavYawDeg,
        double gimbalPanDeg, double gimbalTiltDeg)
    {
        Matrix3x3 rGimbal = buildGimbalRotation(gimbalPanDeg, gimbalTiltDeg);
        Matrix3x3 rBody = buildBodyRotation(uavRollDeg, uavPitchDeg, uavYawDeg);

        return rBody * rGimbal;
    }

    Vector3D GeoCalculator::calculateVWorld(
        double uavRollDeg, double uavPitchDeg, double uavYawDeg,
        double gimbalPanDeg, double gimbalTiltDeg)
    {
        Vector3D vCam(1.0, 0.0, 0.0);

        Matrix3x3 rTotal = calculateCameraToWorldMatrix(
            uavRollDeg, uavPitchDeg, uavYawDeg,
            gimbalPanDeg, gimbalTiltDeg
        );

        return rTotal * vCam;
    }
}