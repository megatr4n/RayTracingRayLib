#pragma once
#include "vec3.h"
#include "ray.h"
#include <algorithm>

namespace rt {

class AABB {
public:
    Vec3 minimum;
    Vec3 maximum;

    AABB() {} 
    
    AABB(const Vec3& min, const Vec3& max) : minimum(min), maximum(max) {}

    AABB(const AABB& box0, const AABB& box1) {
        minimum = Vec3(
            std::min(box0.minimum.x, box1.minimum.x),
            std::min(box0.minimum.y, box1.minimum.y),
            std::min(box0.minimum.z, box1.minimum.z)
        );
        maximum = Vec3(
            std::max(box0.maximum.x, box1.maximum.x),
            std::max(box0.maximum.y, box1.maximum.y),
            std::max(box0.maximum.z, box1.maximum.z)
        );
    }

    bool hit(const Ray& r, float t_min, float t_max) const {
        for (int a = 0; a < 3; a++) {
            float invD = 1.0f / r.direction[a]; 
            float t0 = (minimum[a] - r.origin[a]) * invD;
            float t1 = (maximum[a] - r.origin[a]) * invD;
            
            if (invD < 0.0f) std::swap(t0, t1);
            
            t_min = t0 > t_min ? t0 : t_min;
            t_max = t1 < t_max ? t1 : t_max;
            
            if (t_max <= t_min)
                return false;
        }
        return true;
    }
};

}