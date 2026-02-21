#pragma once
#include "ray.h"
#include "material.h"
#include "aabb.h"
#include <vector>

namespace rt {
struct HitRecord {
    Vec3     p;
    Vec3     normal;
    Material mat;
    float    t = 0;
    float    u;
    float    v;
    bool     frontFace = true;

    void setFaceNormal(const Ray& r, const Vec3& outwardNormal);
};


struct Sphere {
    Vec3     center;
    float    radius = 0.5f;
    Material mat;

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;

    AABB boundingBox() const {
        Vec3 rVec(radius, radius, radius); 
        return AABB(center - rVec, center + rVec);
    }
};

struct Quad {
    Vec3 Q;
    Vec3 u;
    Vec3 v;
    Material mat;

    Vec3 normal;
    float D;
    Vec3 w;

    void init(Vec3 _Q, Vec3 _u, Vec3 _v, Material _mat);
    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;

    AABB boundingBox() const {
        Vec3 p0 = Q;
        Vec3 p1 = Q + u;
        Vec3 p2 = Q + v;
        Vec3 p3 = Q + u + v;

        Vec3 minPoint(
            std::min({p0.x, p1.x, p2.x, p3.x}),
            std::min({p0.y, p1.y, p2.y, p3.y}),
            std::min({p0.z, p1.z, p2.z, p3.z})
        );
        Vec3 maxPoint(
            std::max({p0.x, p1.x, p2.x, p3.x}),
            std::max({p0.y, p1.y, p2.y, p3.y}),
            std::max({p0.z, p1.z, p2.z, p3.z})
        );

        float pad = 0.0001f;
        if (std::abs(maxPoint.x - minPoint.x) < pad) { minPoint.x -= pad; maxPoint.x += pad; }
        if (std::abs(maxPoint.y - minPoint.y) < pad) { minPoint.y -= pad; maxPoint.y += pad; }
        if (std::abs(maxPoint.z - minPoint.z) < pad) { minPoint.z -= pad; maxPoint.z += pad; }

        return AABB(minPoint, maxPoint);
    }
};

class BvhNode;

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Quad>   quads;


    BvhNode* bvhRoot = nullptr;

    ~Scene();
    void buildBVH();             
    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;
};
}