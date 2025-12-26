#include "raylib.h"
#include "../include/vec3.h"
#include "../include/ray.h"
#include "../include/camera.h"
#include "../include/hittable.h"
#include "../include/material.h"
#include <memory>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

bool Lambertian::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    (void)r_in;
    Vec3 scatter_direction = rec.normal + random_unit_vector();
    if (scatter_direction.near_zero())
        scatter_direction = rec.normal;
    scattered = RTRay(rec.p, scatter_direction);
    attenuation = albedo;
    return true;
}

bool Metal::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    Vec3 reflected = reflect(unit_vector(r_in.direction), rec.normal);
    scattered = RTRay(rec.p, reflected + fuzz * random_in_unit_sphere());
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

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > ((double)rand() / RAND_MAX))
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = RTRay(rec.p, direction);
    return true;
}

    Color3 ray_color(const RTRay& r, const HittableList& world, int depth) {
    if (depth <= 0)
        return Color3(0, 0, 0);

    HitRecord rec;
    if (world.hit(r, 0.001, INFINITY, rec)) {
        RTRay scattered;
        Color3 attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return Color3(0, 0, 0);
    }

    Vec3 unit_direction = unit_vector(r.direction);
    double t = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - t) * Color3(1.0, 1.0, 1.0) + t * Color3(0.5, 0.7, 1.0);
}