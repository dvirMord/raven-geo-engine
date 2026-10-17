#pragma once
#include "IGeoCalculator.hpp"
#include "../Common/Constants.hpp"
#include <iostream>

namespace Math 
{

    class GeoCalculator : public IGeoCalculator 
    {
    public:
        GeoCalculator() = default;
        ~GeoCalculator() override = default;

        Models::Ray calculateRay(const Models::TelemetryFrame& frame) override;
    };

} 
