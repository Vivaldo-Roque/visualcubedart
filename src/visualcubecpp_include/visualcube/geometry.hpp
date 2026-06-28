#pragma once
/*
    geometry.hpp — VisualCube C++ Port
    3D geometry helpers: Vec3, translate, scale, rotate, project.
    Direct port of the PHP geometry functions in visualcube.php.

    Copyright (C) 2010 Conrad Rider (original PHP)
    C++ port: same LGPL v3 licence applies.
*/

#include <cmath>
#include <array>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

namespace vc {

struct Vec3 {
    float x{0}, y{0}, z{0};
    float& operator[](int i)       { return i==0?x:i==1?y:z; }
    float  operator[](int i) const { return i==0?x:i==1?y:z; }
};

// Move point by translation vector
inline Vec3 translate(Vec3 p, Vec3 t) {
    return { p.x + t.x, p.y + t.y, p.z + t.z };
}

// Scale all components by scalar factor
inline Vec3 scale(Vec3 p, float f) {
    return { p.x * f, p.y * f, p.z * f };
}

// Scale point relative to position vector v
inline Vec3 trans_scale(Vec3 p, Vec3 v, float f) {
    Vec3 iv{ -v.x, -v.y, -v.z };
    return translate(scale(translate(p, iv), f), v);
}

// Rotate point around axis (0=x, 1=y, 2=z) by angle in radians
inline Vec3 rotate(Vec3 p, int ax, float an) {
    Vec3 np = p;
    float c = std::cos(an);
    float s = std::sin(an);
    switch (ax) {
        case 0: // x-axis
            np.z = p.z * c - p.y * s;
            np.y = p.z * s + p.y * c;
            break;
        case 1: // y-axis
            np.x =  p.x * c + p.z * s;
            np.z = -p.x * s + p.z * c;
            break;
        case 2: // z-axis
            np.x = p.x * c - p.y * s;
            np.y = p.x * s + p.y * c;
            break;
    }
    return np;
}

// Project 3D point onto 2D plane (perspective)
// Maintains z so caller can still use it for depth sorting
inline Vec3 project(Vec3 p, float d) {
    return { p.x * d / p.z, p.y * d / p.z, p.z };
}

} // namespace vc
