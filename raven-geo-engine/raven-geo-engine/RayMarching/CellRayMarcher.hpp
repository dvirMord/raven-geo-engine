#pragma once
#include "IRayMarching.hpp"
#include "../Terrain/IHeightProvider.hpp"
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

	};
}