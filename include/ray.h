#pragma once
#include "vec3.h"

namespace rt {

struct Ray {
    Vec3 origin;
    Vec3 direction;

    Ray() = default;
    Ray(const Vec3& o, const Vec3& d) : origin(o), direction(d) {}

    Vec3 at(float t) const { return origin + direction * t; }
};
}