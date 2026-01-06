#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "hittable.h"
#include "vec3.h"

class Translate : public Hittable {
public:
    Translate(std::shared_ptr<Hittable> p, const Vec3& displacement)
        : object(p), offset(displacement) {
        set_bounding_box();
    }

    bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const override {
        RTRay offset_r(r.origin - offset, r.direction);

        if (!object->hit(offset_r, ray_t, rec))
            return false;

        rec.p += offset;
        return true;
    }

    AABB bounding_box() const override {
        return bbox;
    }

private:
    std::shared_ptr<Hittable> object;
    Vec3 offset;
    AABB bbox;

    void set_bounding_box() {
        AABB old_box = object->bounding_box();

        interval new_x(old_box.x.min + offset.x, old_box.x.max + offset.x);
        interval new_y(old_box.y.min + offset.y, old_box.y.max + offset.y);
        interval new_z(old_box.z.min + offset.z, old_box.z.max + offset.z);

        bbox = AABB(new_x, new_y, new_z);
    }
};

class RotateY : public Hittable {
public:
    RotateY(std::shared_ptr<Hittable> p, double angle) : object(p) {
        auto radians = degrees_to_radians(angle);
        sin_theta = sin(radians);
        cos_theta = cos(radians);
        
        bbox = object->bounding_box();

        Point3 min( infinity,  infinity,  infinity);
        Point3 max(-infinity, -infinity, -infinity);

        for (int i = 0; i < 2; i++) {
            for (int j = 0; j < 2; j++) {
                for (int k = 0; k < 2; k++) {
                    auto x = i * bbox.x.max + (1 - i) * bbox.x.min;
                    auto y = j * bbox.y.max + (1 - j) * bbox.y.min;
                    auto z = k * bbox.z.max + (1 - k) * bbox.z.min;

                    auto newx =  cos_theta * x + sin_theta * z;
                    auto newz = -sin_theta * x + cos_theta * z;

                    Vec3 tester(newx, y, newz);

                    for (int c = 0; c < 3; c++) {
                        min[c] = fmin(min[c], tester[c]);
                        max[c] = fmax(max[c], tester[c]);
                    }
                }
            }
        }

        bbox = AABB(min, max);
    }

    bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const override {
        auto origin = r.origin;
        auto direction = r.direction;

        origin[0] = cos_theta * r.origin[0] - sin_theta * r.origin[2];
        origin[2] = sin_theta * r.origin[0] + cos_theta * r.origin[2];

        direction[0] = cos_theta * r.direction[0] - sin_theta * r.direction[2];
        direction[2] = sin_theta * r.direction[0] + cos_theta * r.direction[2];

        RTRay rotated_r(origin, direction);

        if (!object->hit(rotated_r, ray_t, rec))
            return false;

        auto p = rec.p;
        p[0] =  cos_theta * rec.p[0] + sin_theta * rec.p[2];
        p[2] = -sin_theta * rec.p[0] + cos_theta * rec.p[2];

        auto normal = rec.normal;
        normal[0] =  cos_theta * rec.normal[0] + sin_theta * rec.normal[2];
        normal[2] = -sin_theta * rec.normal[0] + cos_theta * rec.normal[2];

        rec.p = p;
        rec.normal = normal;

        return true;
    }

    AABB bounding_box() const override {
        return bbox;
    }

private:
    std::shared_ptr<Hittable> object;
    double sin_theta;
    double cos_theta;
    AABB bbox;
};

#endif