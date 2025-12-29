#ifndef RT_MATERIAL_H
#define RT_MATERIAL_H

#include "rtweekend.h"
#include "hittable.h"
#include "texture.h" 

class RTMaterial {
public:
    virtual ~RTMaterial() = default;
    virtual bool scatter(
        const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered
    ) const = 0;
};

class Lambertian : public RTMaterial {
public:
    Lambertian(const Color3& albedo);
    Lambertian(std::shared_ptr<Texture> tex);

    bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;

private:
    std::shared_ptr<Texture> tex; 
};

class Metal : public RTMaterial {
public:
    Metal(const Color3& albedo, double fuzz);
    bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;

private:
    Color3 albedo;
    double fuzz;
};

class Dielectric : public RTMaterial {
public:
    Dielectric(double refraction_index);
    bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;

private:
    double refraction_index;

    static double reflectance(double cosine, double ref_idx) {
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

#endif