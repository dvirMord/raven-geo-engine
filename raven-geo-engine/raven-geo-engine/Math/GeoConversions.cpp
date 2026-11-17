#include "GeoConversions.hpp"
#include "../Common/Constants.hpp"
#include <cmath>
#include <numbers>

using std::numbers::pi;

namespace Math
{
	double toRadians(const double degrees)
	{
		return degrees * (pi / Constants::Math::SEMICIRCLE_DEGREES);
	}

	double toDegrees(const double radians)
	{
		return radians * (Constants::Math::SEMICIRCLE_DEGREES / pi);
	}

	DegreesOffset metersToDegrees(const double northMeters, const double eastMeters, const double atLatitudeDegrees)
	{
		DegreesOffset offset;
		offset.latitudeDegrees = toDegrees(northMeters / Constants::Math::EARTH_RADIUS_M);
		offset.longitudeDegrees = toDegrees(eastMeters / (Constants::Math::EARTH_RADIUS_M * std::cos(toRadians(atLatitudeDegrees))));
		return offset;
	}
}
