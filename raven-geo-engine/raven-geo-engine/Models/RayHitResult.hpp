	#pragma once
#include "GroundIntersection.hpp"
#include <optional>
using std::optional;

namespace Models
{
	enum class RayHitStatus
	{
		Hit = 0,
		Miss = 1,
		NoTerrainData = 2
	};

	struct RayHitResult
	{
		RayHitStatus hitStatus = RayHitStatus::Miss;
		optional<GroundIntersection> intersectionPoint = std::nullopt;
	};
}