#pragma once
#include "vec3.h"
#include "ray.h"

struct HitRecord;

enum class MaterialType { Lambertian, Metal, Dielectric };

struct Material {
    MaterialType type   = MaterialType::Lambertian;
    Vec3         albedo = {0.8f, 0.8f, 0.8f};
    float        fuzz   = 0.0f;
    float        ior    = 1.5f;

    bool scatter(const Ray& rIn, const HitRecord& rec,
                 Vec3& attenuation, Ray& scattered) const;
};