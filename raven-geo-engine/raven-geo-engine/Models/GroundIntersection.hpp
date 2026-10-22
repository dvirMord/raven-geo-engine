#pragma once
namespace Models
{
	struct GroundIntersection
	{
		double latitude = 0.0; // Latitude of the intersection point in degrees
		double longitude = 0.0; // Longitude of the intersection point in degrees
		double altitudeMeters = 0.0; // meters above sea level
		double slantRangeMeters = 0.0;// t in the formula - meters from the drone to the point
	};
}