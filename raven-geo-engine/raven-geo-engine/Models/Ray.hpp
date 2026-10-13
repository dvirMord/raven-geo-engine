#pragma once

namespace Models 
{

    struct Vector3D 
    {
        double x = 0.0;
        double y = 0.0;
        double z = 0.0;
    };

    struct Ray 
    {
        Vector3D origin;
        Vector3D direction;
    };
} 
