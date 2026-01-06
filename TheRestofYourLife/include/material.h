#ifndef RT_MATERIAL_H
#define RT_MATERIAL_H

#include "rtweekend.h"
#include "hittable.h"
#include "texture.h" 

class RTMaterial {
public:
    virtual ~RTMaterial() = default;

    virtual Color3 emitted(double u, double v, const Point3& p) const {
        return Color3(0, 0, 0);
    }

    virtual bool scatter(
        const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered
    ) const = 0;
};

class Lambertian : public RTMaterial {
public:
    Lambertian(const Color3& albedo);
    Lambertian(std::shared_ptr<RTTexture> tex); 

    bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override;

private:
    std::shared_ptr<RTTexture> tex; 
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

class DiffuseLight : public RTMaterial {
    public:
        DiffuseLight(std::shared_ptr<RTTexture> tex) : tex(tex) {}
        DiffuseLight(const Color3& emit) : tex(std::make_shared<SolidColor>(emit)) {}
        bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override {
            return false; 
        }
        Color3 emitted(double u, double v, const Point3& p) const override {
            return tex->value(u, v, p);
        }
    private:
        std::shared_ptr<RTTexture> tex;
    };

    class Isotropic : public RTMaterial {
        public:
            Isotropic(const Color3& c) : tex(std::make_shared<SolidColor>(c)) {}
            Isotropic(std::shared_ptr<RTTexture> tex) : tex(tex) {}
            bool scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const override {
                scattered = RTRay(rec.p, random_unit_vector());
                attenuation = tex->value(rec.u, rec.v, rec.p);
                return true;
            }
        private:
            std::shared_ptr<RTTexture> tex;
        };

#endif