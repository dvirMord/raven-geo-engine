#pragma once

namespace Math
{
	// a small move on the map, in degrees
	struct DegreesOffset
	{
		double latitudeDegrees = 0.0;   // positive = north
		double longitudeDegrees = 0.0;  // positive = east
	};

	double toRadians(const double degrees);
	double toDegrees(const double radians);

	// meters north/east → degrees of latitude/longitude, treating the earth as flat around the point
	DegreesOffset metersToDegrees(const double northMeters, const double eastMeters, const double atLatitudeDegrees);
}
