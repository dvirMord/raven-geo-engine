#pragma once
#include <optional>
#include "../Models/RayHitResult.hpp"
#include "../Models/Ray.hpp"
using Models::RayHitResult;
using Models::RayHitStatus;
using Models::Ray;

namespace RayMarching
{
	class IRayMarching
	{
	public:
		virtual ~IRayMarching() = default;
		// returns the first hit point of the ray with the terrain, or a miss if it doesn't hit any terrain
		virtual RayHitResult getFirstHit(const Ray& ray) const = 0;

	};
}