#ifndef RT_RAY_H
#define RT_RAY_H

#include "vec3.h"

class RTRay {
public:
    Point3 origin;
    Vec3 direction;

    RTRay() {}
    RTRay(const Point3& origin, const Vec3& direction)
        : origin(origin), direction(direction) {}

    Point3 at(double t) const {
        return origin + t * direction;
    }
};

#endif // RT_RAY_H