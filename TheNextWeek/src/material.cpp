#include "../include/material.h"
#include "../include/hittable.h"
#include "../include/rtweekend.h" 

bool Lambertian::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    Vec3 scatter_direction = rec.normal + random_unit_vector();

    if (scatter_direction.near_zero())
        scatter_direction = rec.normal;

    scattered = RTRay(rec.p, scatter_direction, r_in.tm);
    attenuation = albedo;
    return true;
}

bool Metal::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    Vec3 reflected = reflect(unit_vector(r_in.direction), rec.normal);
    
    scattered = RTRay(rec.p, reflected + fuzz * random_in_unit_sphere(), r_in.tm);
    attenuation = albedo;
    
    return (dot(scattered.direction, rec.normal) > 0);
}

bool Dielectric::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    attenuation = Color3(1.0, 1.0, 1.0); 
    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

    Vec3 unit_direction = unit_vector(r_in.direction);
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    Vec3 direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > random_double())
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = RTRay(rec.p, direction, r_in.tm);
    return true;
}