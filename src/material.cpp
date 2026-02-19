#include "material.h"
#include "hittable.h"
#include "random_utils.h"
#include <cmath>

namespace rt
{

    static float schlick(float cosine, float ior)
    {
        float r0 = (1.0f - ior) / (1.0f + ior);
        r0 = r0 * r0;
        return r0 + (1.0f - r0) * std::pow((1.0f - cosine), 5.0f);
    }

    float reflectance(float cosine, float ref_idx)
    {
        float r0 = (1.0f - ref_idx) / (1.0f + ref_idx);
        r0 = r0 * r0;
        return r0 + (1.0f - r0) * std::pow((1.0f - cosine), 5.0f);
    }

    bool Material::scatter(const Ray &rIn, const HitRecord &rec,
                           Vec3 &attenuation, Ray &scattered) const
    {

        if (type == MaterialType::DiffuseLight)
        {
            return false;
        }

        if (type == MaterialType::Lambertian)
        {
            Vec3 scatter_dir = rec.normal + randomUnitVector();
            if (scatter_dir.lengthSq() < 1e-8)
            {
                scatter_dir = rec.normal;
            }
            scattered = Ray(rec.p, scatter_dir);
            attenuation = albedo;
            return true;
        }

        if (type == MaterialType::Metal)
        {
            Vec3 reflected = reflect(normalize(rIn.direction), rec.normal);
            scattered = Ray(rec.p, reflected + fuzz * randomInUnitSphere());
            attenuation = albedo;
            return (dot(scattered.direction, rec.normal) > 0);
        }

        if (type == MaterialType::Dielectric)
        {
            attenuation = {1.0f, 1.0f, 1.0f};
            float refraction_ratio = rec.frontFace ? (1.0f / ior) : ior;
            Vec3 unit_direction = normalize(rIn.direction);

            float cos_theta = std::fmin(dot(-unit_direction, rec.normal), 1.0f);
            float sin_theta = std::sqrt(1.0f - cos_theta * cos_theta);

            bool cannot_refract = refraction_ratio * sin_theta > 1.0f;
            Vec3 direction;
            if (cannot_refract || reflectance(cos_theta, refraction_ratio) > randomFloat())
            {
                direction = reflect(unit_direction, rec.normal);
            }
            else
            {
                direction = refract(unit_direction, rec.normal, refraction_ratio);
            }

            scattered = Ray(rec.p, direction);
            return true;
        }

        return false;
    }

}