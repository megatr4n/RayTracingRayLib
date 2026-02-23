#pragma once
#include "aabb.h"
#include "hittable.h"
#include <vector>
#include <algorithm>

namespace rt {

struct BvhItem {
    AABB box;
    const Sphere* sphere = nullptr;
    const Quad* quad = nullptr;
    const Triangle* tri = nullptr;
    const Mesh* mesh = nullptr;
};

class BvhNode {
public:
    AABB box;
    BvhNode* left = nullptr;
    BvhNode* right = nullptr;
    
    const Sphere* sphere = nullptr;
    const Quad* quad = nullptr;

    BvhNode(std::vector<BvhItem>& objects, size_t start, size_t end) {
        AABB bounds = objects[start].box;
        for (size_t i = start + 1; i < end; i++) {
            bounds = AABB(bounds, objects[i].box);
        }
        Vec3 extent = bounds.maximum - bounds.minimum;
        int axis = 0; // 0 = X, 1 = Y, 2 = Z
        if (extent.y > extent.x && extent.y > extent.z) axis = 1;
        else if (extent.z > extent.x && extent.z > extent.y) axis = 2;

        auto comparator = [axis](const BvhItem& a, const BvhItem& b) {
            return a.box.minimum[axis] < b.box.minimum[axis];
        };

        size_t object_span = end - start;

        if (object_span == 1) {
            sphere = objects[start].sphere;
            quad = objects[start].quad;
            box = objects[start].box;
        } else if (object_span == 2) {
            if (comparator(objects[start], objects[start+1])) {
                left = new BvhNode(objects, start, start + 1);
                right = new BvhNode(objects, start + 1, end);
            } else {
                left = new BvhNode(objects, start + 1, end);
                right = new BvhNode(objects, start, start + 1);
            }
            box = AABB(left->box, right->box);
        } else {
            std::sort(objects.begin() + start, objects.begin() + end, comparator);
            size_t mid = start + object_span / 2;
            left = new BvhNode(objects, start, mid);
            right = new BvhNode(objects, mid, end);
            box = AABB(left->box, right->box);
        }
    }

    ~BvhNode() {
        delete left;
        delete right;
    }

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
        if (!box.hit(r, tMin, tMax)) 
            return false;

        if (sphere) return sphere->hit(r, tMin, tMax, rec);
        if (quad) return quad->hit(r, tMin, tMax, rec);

        bool hitLeft = left && left->hit(r, tMin, tMax, rec);
        bool hitRight = right && right->hit(r, tMin, hitLeft ? rec.t : tMax, rec);
        
        return hitLeft || hitRight;
    }
};

}