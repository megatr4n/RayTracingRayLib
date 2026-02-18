#pragma once
#include "ray.h"
#include "material.h"
#include <vector>

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

struct Scene {
    std::vector<Sphere> spheres;

    bool hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const;
};