#pragma once
#include "vec3.h"
#include "ray.h"
#include "texture.h"

namespace rt
{
    struct HitRecord;

    enum class MaterialType
    {
        Lambertian,
        Metal,
        Dielectric,
        DiffuseLight
    };

    struct Material
    {
        MaterialType type = MaterialType::Lambertian;
        Vec3 albedo = {0.8f, 0.8f, 0.8f};
        Vec3 emit = {0.0f, 0.0f, 0.0f};
        float fuzz = 0.0f;
        float ior = 1.5f;

        std::shared_ptr<Texture> tex = nullptr;

        bool scatter(const Ray &rIn, const HitRecord &rec,
                     Vec3 &attenuation, Ray &scattered) const;

        Vec3 emitted(float u, float v, const Vec3 &p) const
        {
            if (type == MaterialType::DiffuseLight)
            {
                if (tex)
                    return tex->value(u, v, p);
                return emit;
            }
            return {0, 0, 0};
        }
    };
}