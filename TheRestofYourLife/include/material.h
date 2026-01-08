#ifndef RT_MATERIAL_H
#define RT_MATERIAL_H

#include "rtweekend.h"
#include "texture.h" 

#include "hittable.h"

class Pdf; 

struct ScatterRecord {
    Color3 attenuation;
    std::shared_ptr<Pdf> pdf_ptr;
    bool skip_pdf;      
    RTRay skip_pdf_ray; 
};

class RTMaterial {
public:
    virtual ~RTMaterial() = default;
    
    virtual Color3 emitted(const RTRay& r_in, const HitRecord& rec, double u, double v, const Point3& p) const {
        return Color3(0, 0, 0);
    }
    
    virtual bool scatter(
        const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec
    ) const = 0;
    
    virtual double scattering_pdf(const RTRay& r_in, const HitRecord& rec, const RTRay& scattered) const {
        return 0;
    }
};

class Lambertian : public RTMaterial {
public:
    Lambertian(const Color3& albedo);
    Lambertian(std::shared_ptr<RTTexture> tex); 

    bool scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const override;
    double scattering_pdf(const RTRay& r_in, const HitRecord& rec, const RTRay& scattered) const override;

private:
    std::shared_ptr<RTTexture> tex; 
};

class Metal : public RTMaterial {
public:
    Metal(const Color3& albedo, double fuzz);
    bool scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const override;

private:
    Color3 albedo;
    double fuzz;
};

class Dielectric : public RTMaterial {
public:
    Dielectric(double refraction_index);
    bool scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const override;

private:
    double refraction_index;

    static double reflectance(double cosine, double ref_idx) {
        auto r0 = (1 - ref_idx) / (1 + ref_idx);
        r0 = r0 * r0;
        return r0 + (1 - r0) * pow((1 - cosine), 5);
    }
};

class DiffuseLight : public RTMaterial {
public:
    DiffuseLight(std::shared_ptr<RTTexture> tex) : tex(tex) {}
    DiffuseLight(const Color3& emit) : tex(std::make_shared<SolidColor>(emit)) {}

    bool scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const override {
        return false;
    }

    Color3 emitted(const RTRay& r_in, const HitRecord& rec, double u, double v, const Point3& p) const override;

private:
    std::shared_ptr<RTTexture> tex;
};

#endif