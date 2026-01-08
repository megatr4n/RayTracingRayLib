#include "material.h"
#include "hittable.h" 
#include "pdf.h"      
#include "texture.h"  

Color3 DiffuseLight::emitted(const RTRay& r_in, const HitRecord& rec, double u, double v, const Point3& p) const {
    if (!rec.front_face) return Color3(0,0,0);
    return tex->value(u, v, p);
}

Lambertian::Lambertian(const Color3& albedo) : tex(std::make_shared<SolidColor>(albedo)) {}
Lambertian::Lambertian(std::shared_ptr<RTTexture> tex) : tex(tex) {}

bool Lambertian::scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const {
    srec.attenuation = tex->value(rec.u, rec.v, rec.p);
    srec.pdf_ptr = std::make_shared<CosinePdf>(rec.normal);
    srec.skip_pdf = false;
    return true;
}

double Lambertian::scattering_pdf(const RTRay& r_in, const HitRecord& rec, const RTRay& scattered) const {
    auto cos_theta = dot(rec.normal, unit_vector(scattered.direction));
    return cos_theta < 0 ? 0 : cos_theta / pi;
}

Metal::Metal(const Color3& albedo, double fuzz) : albedo(albedo), fuzz(fuzz < 1 ? fuzz : 1) {}

bool Metal::scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const {
    Vec3 reflected = reflect(unit_vector(r_in.direction), rec.normal);
    
    srec.attenuation = albedo;
    srec.skip_pdf = true;
    srec.skip_pdf_ray = RTRay(rec.p, reflected + fuzz * random_unit_vector(), r_in.tm);

    if (dot(srec.skip_pdf_ray.direction, rec.normal) <= 0)
        return false;
    
    return true;
}

Dielectric::Dielectric(double refraction_index) : refraction_index(refraction_index) {}

bool Dielectric::scatter(const RTRay& r_in, const HitRecord& rec, ScatterRecord& srec) const {
    srec.attenuation = Color3(1.0, 1.0, 1.0);
    srec.skip_pdf = true;

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

    srec.skip_pdf_ray = RTRay(rec.p, direction, r_in.tm);
    return true;
}