#include "../include/material.h"


    Lambertian::Lambertian(const Color3& albedo) : tex(std::make_shared<SolidColor>(albedo)) {}

    Lambertian::Lambertian(std::shared_ptr<RTTexture> tex) : tex(tex) {}

    bool Lambertian::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
        auto scatter_direction = rec.normal + random_unit_vector();

        if (scatter_direction.near_zero())
            scatter_direction = rec.normal;
        scattered = RTRay(rec.p, scatter_direction, r_in.tm);
        attenuation = tex->value(rec.u, rec.v, rec.p);
        return true;
    }

    Metal::Metal(const Color3& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

    bool Metal::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
        Vec3 reflected = reflect(unit_vector(r_in.direction), rec.normal);
        scattered = RTRay(rec.p, reflected + fuzz * random_unit_vector(), r_in.tm);
        attenuation = albedo;
        return (dot(scattered.direction, rec.normal) > 0);
    }


    Dielectric::Dielectric(double refraction_index) : refraction_index(refraction_index) {}

    bool Dielectric::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
        attenuation = Color3(1.0, 1.0, 1.0);
        
        double ri = rec.front_face ? (1.0 / refraction_index) : refraction_index;

        Vec3 unit_direction = unit_vector(r_in.direction);
        double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
        double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

        bool cannot_refract = ri * sin_theta > 1.0;
        Vec3 direction;

        if (cannot_refract || reflectance(cos_theta, ri) > random_double())
            direction = reflect(unit_direction, rec.normal);
        else
            direction = refract(unit_direction, rec.normal, ri);

        scattered = RTRay(rec.p, direction, r_in.tm);
        return true;
    }