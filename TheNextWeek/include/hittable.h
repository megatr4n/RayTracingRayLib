#ifndef RT_HITTABLE_H
#define RT_HITTABLE_H

#include "ray.h"
#include "vec3.h"
#include <memory>
#include <vector>
#include <cmath>

class RTMaterial;

struct HitRecord {
    Point3 p;
    Vec3 normal;
    std::shared_ptr<RTMaterial> mat;
    double t;
    bool front_face;

    void set_face_normal(const RTRay& r, const Vec3& outward_normal) {
        front_face = dot(r.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual bool hit(const RTRay& r, double t_min, double t_max, HitRecord& rec) const = 0;
    virtual ~Hittable() = default;
};

class Sphere : public Hittable {
public:
    Point3 center;
    double radius;
    std::shared_ptr<RTMaterial> mat;

    Sphere(Point3 center, double radius, std::shared_ptr<RTMaterial> mat)
        : center(center), radius(radius), mat(mat) {}

    virtual bool hit(const RTRay& r, double t_min, double t_max, HitRecord& rec) const override {
        Vec3 oc = r.origin - center;
        double a = r.direction.length_squared();
        double half_b = dot(oc, r.direction);
        double c = oc.length_squared() - radius * radius;

        double discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;
        double sqrtd = std::sqrt(discriminant);

        double root = (-half_b - sqrtd) / a;
        if (root <= t_min || t_max <= root) {
            root = (-half_b + sqrtd) / a;
            if (root <= t_min || t_max <= root)
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        Vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);
        rec.mat = mat;

        return true;
    }
};

class HittableList : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;

    HittableList() {}
    HittableList(std::shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }
    void add(std::shared_ptr<Hittable> object) { objects.push_back(object); }

    virtual bool hit(const RTRay& r, double t_min, double t_max, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        double closest_so_far = t_max;

        for (const auto& object : objects) {
            if (object->hit(r, t_min, closest_so_far, temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
};

#endif // RT_HITTABLE_H