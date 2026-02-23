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

struct Triangle {
    Vec3 v0, v1, v2;
    Material mat;

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;

    AABB boundingBox() const {
        Vec3 minPoint(
            std::min({v0.x, v1.x, v2.x}),
            std::min({v0.y, v1.y, v2.y}),
            std::min({v0.z, v1.z, v2.z})
        );
        Vec3 maxPoint(
            std::max({v0.x, v1.x, v2.x}),
            std::max({v0.y, v1.y, v2.y}),
            std::max({v0.z, v1.z, v2.z})
        );

        float pad = 0.0001f;
        if (std::abs(maxPoint.x - minPoint.x) < pad) { minPoint.x -= pad; maxPoint.x += pad; }
        if (std::abs(maxPoint.y - minPoint.y) < pad) { minPoint.y -= pad; maxPoint.y += pad; }
        if (std::abs(maxPoint.z - minPoint.z) < pad) { minPoint.z -= pad; maxPoint.z += pad; }

        return AABB(minPoint, maxPoint);
    }
};

struct Mesh {
    Vec3 position;
    std::vector<Triangle> localTriangles;
    Material mat;  

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
        Ray localRay(r.origin - position, r.direction);
        bool hitAny = false;
        float best = tMax;
        HitRecord tmp;

        for (const auto& tri : localTriangles) {
            if (tri.hit(localRay, tMin, best, tmp)) {
                hitAny = true;
                best = tmp.t;
                rec = tmp;
            }
        }
        if (hitAny) {
            rec.p = rec.p + position; 
            rec.mat = mat;           
        }
        return hitAny;
    }

    AABB boundingBox() const {
        if (localTriangles.empty()) return AABB(position, position);

        Vec3 minPoint = localTriangles[0].v0;
        Vec3 maxPoint = localTriangles[0].v0;

        for (const auto& tri : localTriangles) {
            for (const auto& v : {tri.v0, tri.v1, tri.v2}) {
                minPoint.x = std::min(minPoint.x, v.x);
                minPoint.y = std::min(minPoint.y, v.y);
                minPoint.z = std::min(minPoint.z, v.z);

                maxPoint.x = std::max(maxPoint.x, v.x);
                maxPoint.y = std::max(maxPoint.y, v.y);
                maxPoint.z = std::max(maxPoint.z, v.z);
            }
        }

        minPoint = minPoint + position;
        maxPoint = maxPoint + position;

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
    std::vector<Triangle> triangles;
    std::vector<Mesh> meshes;

    BvhNode* bvhRoot = nullptr;

    ~Scene();
    void buildBVH();             
    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;
};
}