#pragma once
#include "IRayMarching.hpp"
#include "../Terrain/IHeightProvider.hpp"
#include "../Math/GeoConversions.hpp"
#include <optional>
using std::optional;
using Terrain::IHeightProvider;

namespace RayMarching
{
	class CellRayMarcher : public IRayMarching
	{
	public:
		explicit CellRayMarcher(const IHeightProvider& heightProvider);
		~CellRayMarcher() override = default;
		RayHitResult getFirstHit(const Ray& ray) const override;

	private:
		const IHeightProvider& _heightProvider;

		// meters from the drone to where the ray leaves this cell - the nearer of the two walls it faces
		double metersToCellExit(const TerrainCell& cell, const Ray& ray, const Math::DegreesOffset& degreesPerMeter) const;
		// meters from the drone to where the ray first goes below the cell's height - nullopt if it passes over the cell
		optional<double> metersToCellHit(const TerrainCell& cell, const Ray& ray, const double tEntry, const double tExit) const;

	};
}