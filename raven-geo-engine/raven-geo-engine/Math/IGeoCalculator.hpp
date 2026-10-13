#pragma once
#include "../Models/TelemetryFrame.hpp"
#include "../Models/Ray.hpp"

namespace Math 
{
    class IGeoCalculator 
    {
    public:
        virtual ~IGeoCalculator() = default;
        virtual Models::Ray calculateRay(const Models::TelemetryFrame& frame) = 0;
    };
}