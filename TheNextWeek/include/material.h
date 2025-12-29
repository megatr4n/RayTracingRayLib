#ifndef RT_MATERIAL_H
#define RT_MATERIAL_H

#include "ray.h"
#include "vec3.h"
#include <cmath>

struct HitRecord;

class RTMaterial {
public:
    virtual bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const = 0;
    virtual ~RTMaterial() = default;
};

class Lambertian : public RTMaterial {
public:
    Color3 albedo;

    Lambertian(const Color3& a) : albedo(a) {}

    virtual bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;
};

class Metal : public RTMaterial {
public:
    Color3 albedo;
    double fuzz;

    Metal(const Color3& a, double f) : albedo(a), fuzz(f < 1 ? f : 1) {}

    virtual bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;
};

class Dielectric : public RTMaterial {
public:
    double ir;

    Dielectric(double index_of_refraction) : ir(index_of_refraction) {}

    virtual bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;

private:
    static double reflectance(double cosine, double ref_idx) {
        double r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * std::pow((1 - cosine), 5);
    }
};

#endif 