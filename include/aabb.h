#pragma once
#include "vec3.h"
#include "ray.h"
#include <algorithm>

namespace rt {

class AABB {
public:
    Vec3 minimum = {1e9f, 1e9f, 1e9f};
    Vec3 maximum = {-1e9f, -1e9f, -1e9f};

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

    void extend(const AABB& b) {
        minimum.x = std::min(minimum.x, b.minimum.x);
        minimum.y = std::min(minimum.y, b.minimum.y);
        minimum.z = std::min(minimum.z, b.minimum.z);
        maximum.x = std::max(maximum.x, b.maximum.x);
        maximum.y = std::max(maximum.y, b.maximum.y);
        maximum.z = std::max(maximum.z, b.maximum.z);
    }

    Vec3 centroid() const {
        return minimum + (maximum - minimum) * 0.5f;
    }

    float area() const {
        Vec3 ext = maximum - minimum;
        if (ext.x <= 0 || ext.y <= 0 || ext.z <= 0) return 0.0f;
        return 2.0f * (ext.x * ext.y + ext.y * ext.z + ext.z * ext.x);
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