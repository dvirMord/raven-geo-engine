#pragma once
#include <optional>
using std::optional;


namespace Terrain
{
	class IHeightProvider
	{
	public:
		virtual ~IHeightProvider() = default;
		// ground height in meters above sea level at the point - nullopt when no loaded tile covers it
		virtual optional<double> getHeightMeters(const double latitude, const double longitude) const = 0;
	};
}