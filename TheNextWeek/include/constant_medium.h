#ifndef CONSTANT_MEDIUM_H
#define CONSTANT_MEDIUM_H

#include "hittable.h"
#include "material.h"
#include "texture.h"
#include "rtweekend.h" 

class ConstantMedium : public Hittable {
public:
    ConstantMedium(std::shared_ptr<Hittable> b, double d, std::shared_ptr<RTTexture> tex)
        : boundary(b), neg_inv_density(-1/d), phase_function(std::make_shared<Isotropic>(tex)) {}

    ConstantMedium(std::shared_ptr<Hittable> b, double d, const Color3& c)
        : boundary(b), neg_inv_density(-1/d), phase_function(std::make_shared<Isotropic>(c)) {}

    bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const override {
        HitRecord rec1, rec2;

        if (!boundary->hit(r, interval(-infinity, infinity), rec1))
            return false;
        if (!boundary->hit(r, interval(rec1.t + 0.0001, infinity), rec2))
            return false;
        if (rec1.t < ray_t.min) rec1.t = ray_t.min;
        if (rec2.t > ray_t.max) rec2.t = ray_t.max;
        if (rec1.t >= rec2.t)
            return false;
        if (rec1.t < 0)
            rec1.t = 0;

        auto ray_length = r.direction.length();
        auto distance_inside_boundary = (rec2.t - rec1.t) * ray_length;
        auto hit_distance = neg_inv_density * log(random_double());

        if (hit_distance > distance_inside_boundary)
            return false;

        rec.t = rec1.t + hit_distance / ray_length;
        rec.p = r.at(rec.t);

        rec.normal = Vec3(1,0,0);
        rec.front_face = true;    
        rec.mat = phase_function;

        return true;
    }

    AABB bounding_box() const override {
        return boundary->bounding_box();
    }

private:
    std::shared_ptr<Hittable> boundary;
    double neg_inv_density;
    std::shared_ptr<RTMaterial> phase_function;

};


#endif