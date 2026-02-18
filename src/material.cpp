#include "material.h"
#include "hittable.h"
#include "random_utils.h"
#include <cmath>


static float schlick(float cosine, float ior) {
    float r0 = (1.0f - ior) / (1.0f + ior);
    r0 = r0 * r0;
    return r0 + (1.0f - r0) * std::pow((1.0f - cosine), 5.0f);
}


bool Material::scatter(const Ray& rIn, const HitRecord& rec,
                       Vec3& attenuation, Ray& scattered) const
{
    switch (type) {

    case MaterialType::Lambertian: {
        Vec3 dir = rec.normal + randomUnitVector();
        if (dir.nearZero()) dir = rec.normal;
        scattered   = Ray(rec.p, normalize(dir));
        attenuation = albedo;
        return true;
    }

    case MaterialType::Metal: {
        Vec3 reflected = reflect(normalize(rIn.direction), rec.normal);
        reflected   = reflected + fuzz * randomInUnitSphere();
        scattered   = Ray(rec.p, normalize(reflected));
        attenuation = albedo;
        return dot(scattered.direction, rec.normal) > 0;
    }

    case MaterialType::Dielectric: {
        attenuation  = {1.0f, 1.0f, 1.0f};
        float ratio  = rec.frontFace ? (1.0f / ior) : ior;

        Vec3  unitDir  = normalize(rIn.direction);
        float cosTheta = std::fmin(dot(-unitDir, rec.normal), 1.0f);
        float sinTheta = std::sqrt(1.0f - cosTheta * cosTheta);

        bool cannotRefract = ratio * sinTheta > 1.0f;
        Vec3 dir;
        if (cannotRefract || schlick(cosTheta, ratio) > randomFloat())
            dir = reflect(unitDir, rec.normal);
        else
            dir = refract(unitDir, rec.normal, ratio);

        scattered = Ray(rec.p, dir);
        return true;
    }

    }
    return false;
}