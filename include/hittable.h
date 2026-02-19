#pragma once
#include "ray.h"
#include "material.h"
#include <vector>

namespace rt {
struct HitRecord {
    Vec3     p;
    Vec3     normal;
    Material mat;
    float    t         = 0;
    bool     frontFace = true;

    void setFaceNormal(const Ray& r, const Vec3& outwardNormal);
};

struct Sphere {
    Vec3     center;
    float    radius = 0.5f;
    Material mat;

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;
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
};

struct Scene {
    std::vector<Sphere> spheres;
    std::vector<Quad>   quads;

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;
};
}