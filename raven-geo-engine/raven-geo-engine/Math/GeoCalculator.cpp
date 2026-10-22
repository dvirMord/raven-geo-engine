#include "GeoCalculator.hpp"
#include <cmath>
#include <spdlog/spdlog.h>
#include <numbers>

using std::numbers::pi;
using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

namespace Math 
{
    Models::Ray GeoCalculator::calculateRay(const Models::TelemetryFrame& frame) 
    {
        Models::Ray ray;

        // findMissingRayFields must pass before this, so every value exists
        ray.direction = this->calculateVWorld(
            frame.platformRollDeg.value(),
            frame.platformPitchDeg.value(),
            frame.platformHeadingDeg.value(),
            frame.gimbalAzimuthDeg.value(),
            frame.gimbalElevationDeg.value()
        );

        ray.origin = RavenMaths::Vector3D(
            frame.sensorLatitude.value(),
            frame.sensorLongitude.value(),
            frame.sensorAltitudeMeters.value()
        );
        spdlog::info(Constants::Logs::CALC_RAY, ray.origin.toString(), ray.direction.toString());
        return ray;
    }

    std::vector<std::string> GeoCalculator::findMissingRayFields(const Models::TelemetryFrame& frame)
    {
        std::vector<std::string> missingFields;

        if (!frame.sensorLatitude) missingFields.push_back(Constants::TelemetryJsonKeys::SENSOR_LATITUDE);
        if (!frame.sensorLongitude) missingFields.push_back(Constants::TelemetryJsonKeys::SENSOR_LONGITUDE);
        if (!frame.sensorAltitudeMeters) missingFields.push_back(Constants::TelemetryJsonKeys::SENSOR_ALTITUDE_METERS);
        if (!frame.platformHeadingDeg) missingFields.push_back(Constants::TelemetryJsonKeys::PLATFORM_HEADING_DEG);
        if (!frame.platformPitchDeg) missingFields.push_back(Constants::TelemetryJsonKeys::PLATFORM_PITCH_DEG);
        if (!frame.platformRollDeg) missingFields.push_back(Constants::TelemetryJsonKeys::PLATFORM_ROLL_DEG);
        if (!frame.gimbalAzimuthDeg) missingFields.push_back(Constants::TelemetryJsonKeys::GIMBAL_AZIMUTH_DEG);
        if (!frame.gimbalElevationDeg) missingFields.push_back(Constants::TelemetryJsonKeys::GIMBAL_ELEVATION_DEG);

        return missingFields;
    }

	double GeoCalculator::toRadians(const double degrees)
	{
		return degrees * (pi / Constants::Math::SEMICIRCLE_DEGREES);
	}
	double GeoCalculator::toDegrees(const double radians)
	{
		return radians * (Constants::Math::SEMICIRCLE_DEGREES / pi);
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

    std::optional<Models::GroundIntersection> GeoCalculator::castRayToFlatGround(const Models::Ray& ray, double groundAltitudeMeters)
    {
		// checking if direction vector looks above the drone(like to the sky)
        if (ray.direction.getZ() <= 0)
        {
			spdlog::warn(Constants::Logs::CALC_Z_POINT_WARNING, ray.direction.toString());
            return  std::nullopt; 
        }

		// in case the drone is below the ground level, we can't calculate the intersection point
        if(ray.origin.getZ() <= groundAltitudeMeters)
        {
			spdlog::warn(Constants::Logs::DRONE_BELOW_GROUND_WARNING, ray.origin.getZ(), groundAltitudeMeters);
            return std::nullopt;
        }

        // the t in the formula - distance from the hit point 
        double slantRangeMeters = (ray.origin.getZ() - groundAltitudeMeters) / ray.direction.getZ();

		//moving the direction vector to the ground intersection point
        double northMeters = slantRangeMeters * ray.direction.getX();
		double eastMeters = slantRangeMeters * ray.direction.getY();

		// converting the north/east meters to latitude/longitude
        double deltaLat = toDegrees(northMeters / Constants::Math::EARTH_RADIUS_M);
        double deltaLon = toDegrees(eastMeters / (Constants::Math::EARTH_RADIUS_M * std::cos(toRadians(ray.origin.getX()))));

		return  Models::GroundIntersection{
			ray.origin.getX() + deltaLat,
			ray.origin.getY() + deltaLon,
			groundAltitudeMeters,
            slantRangeMeters
		};  
    }

    double GeoCalculator::haversineDistanceMeters(const double lat1, const double lon1, const double lat2, const double lon2)
    {
        double phi1 = toRadians(lat1);
        double phi2 = toRadians(lat2);

        double deltaPhi = toRadians(lat2 - lat1);
        double deltaLambda = toRadians(lon2 - lon1);

        double a = std::sin(deltaPhi / 2) * std::sin(deltaPhi / 2) +
			std::cos(phi1) * std::cos(phi2) *
			std::sin(deltaLambda / 2) * std::sin(deltaLambda / 2);
        double c = 2 * std::atan2(std::sqrt(a), std::sqrt(1 - a));
		return Constants::Math::EARTH_RADIUS_M * c;
    }
}   