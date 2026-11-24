#pragma once
#include "IRayMarching.hpp"
#include "../Terrain/IHeightProvider.hpp"
#include "../Math/GeoConversions.hpp"
#include "../Models/Bounds.hpp"
#include <optional>
using std::optional;
using Models::Bounds;
using Terrain::IHeightProvider;

namespace RayMarching
{
	class BlockSkipRayMarcher : public IRayMarching
	{
	public:
		explicit BlockSkipRayMarcher(const IHeightProvider& heightProvider);
		~BlockSkipRayMarcher() override = default;
		RayHitResult getFirstHit(const Ray& ray) const override;

	private:
		const IHeightProvider& _heightProvider;

		// meters from the drone to where the ray leaves this cell/block - the nearer of the two walls it faces
		double metersToExit(const Bounds& bounds, const Ray& ray, const Math::DegreesOffset& degreesPerMeter) const;
		// meters from the drone to where the ray first goes below the cell's height - nullopt if it passes over the cell
		optional<double> metersToCellHit(const TerrainCell& cell, const Ray& ray, const double tEntry, const double tExit) const;
		// meters from the drone to the exit of the block at this point, if the ray stays above the block's max height until then - nullopt if it can't skip it
		optional<double> metersToBlockSkip(const double latitude, const double longitude, const Ray& ray, const Math::DegreesOffset& degreesPerMeter) const;

	};
}