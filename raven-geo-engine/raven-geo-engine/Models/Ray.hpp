#pragma once
#include "Vector3D.hpp"
#include "Matrix3x3.hpp"

using RavenMaths::Matrix3x3;
using RavenMaths::Vector3D;

namespace Models 
{
    struct Ray 
    {
        Vector3D origin;
        Vector3D direction;
    };
} 
