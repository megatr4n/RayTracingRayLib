#pragma once
#include "vec3.h"
#include "ray.h"

namespace rt {
struct HitRecord;

enum class MaterialType { Lambertian, Metal, Dielectric, DiffuseLight };

struct Material {
    MaterialType type   = MaterialType::Lambertian;
    Vec3         albedo = {0.8f, 0.8f, 0.8f};
    Vec3         emit   = {0.0f, 0.0f, 0.0f};
    float        fuzz   = 0.0f;
    float        ior    = 1.5f;

    bool scatter(const Ray& rIn, const HitRecord& rec,
                 Vec3& attenuation, Ray& scattered) const;

    Vec3 emitted() const {
        if (type == MaterialType::DiffuseLight) return emit;
        return {0, 0, 0};
    }
};
}