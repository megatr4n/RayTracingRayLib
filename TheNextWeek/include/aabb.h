#ifndef RT_AABB_H
#define RT_AABB_H

#include "rtweekend.h"
#include "interval.h"
#include "vec3.h"

class AABB {
public:
    interval x, y, z;

    AABB() {} 
    AABB(const interval& ix, const interval& iy, const interval& iz)
        : x(ix), y(iy), z(iz) {}

    AABB(const Point3& a, const Point3& b) {
        x = interval(fmin(a.x, b.x), fmax(a.x, b.x));
        y = interval(fmin(a.y, b.y), fmax(a.y, b.y));
        z = interval(fmin(a.z, b.z), fmax(a.z, b.z));
    }

    AABB(const AABB& box0, const AABB& box1) {
        x = interval(box0.x, box1.x);
        y = interval(box0.y, box1.y);
        z = interval(box0.z, box1.z);
    }

    const interval& axis(int n) const {
        if (n == 1) return y;
        if (n == 2) return z;
        return x;
    }

    bool hit(const RTRay& r, interval ray_t) const {
        for (int a = 0; a < 3; a++) {
            const auto& ax = axis(a);
            const double adinv = 1.0 / r.direction[a];

            auto t0 = (ax.min - r.origin[a]) * adinv;
            auto t1 = (ax.max - r.origin[a]) * adinv;

            if (t0 < t1) {
                if (t0 > ray_t.min) ray_t.min = t0;
                if (t1 < ray_t.max) ray_t.max = t1;
            } else {
                if (t1 > ray_t.min) ray_t.min = t1;
                if (t0 < ray_t.max) ray_t.max = t0;
            }

            if (ray_t.max <= ray_t.min)
                return false;
        }
        return true;
    }
};

#endif 