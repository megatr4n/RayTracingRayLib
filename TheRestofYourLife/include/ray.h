#ifndef RT_RAY_H
#define RT_RAY_H

#include "vec3.h"

class RTRay {
public:
    Point3 origin;
    Vec3 direction;
    double tm;

    RTRay() {}
    RTRay(const Point3& origin, const Vec3& direction, double tm = 0.0)
        : origin(origin), direction(direction), tm(tm) {}

    double time() const { return tm; }

    Point3 at(double t) const {
        return origin + t * direction;
    }
};

#endif