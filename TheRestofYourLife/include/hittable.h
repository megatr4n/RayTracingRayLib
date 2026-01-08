#ifndef RT_HITTABLE_H
#define RT_HITTABLE_H

#include "rtweekend.h" 
#include "aabb.h"     
#include "ray.h"
#include "interval.h"  

#include <memory>
#include <vector>

class RTMaterial;

struct HitRecord {
    Point3 p;
    Vec3 normal;
    std::shared_ptr<RTMaterial> mat;
    double t;
    double u; 
    double v;
    bool front_face;

    void set_face_normal(const RTRay& r, const Vec3& outward_normal) {
        front_face = dot(r.direction, outward_normal) < 0;
        normal = front_face ? outward_normal : -outward_normal;
    }
};

class Hittable {
public:
    virtual ~Hittable() = default;

    virtual bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const = 0;

    virtual AABB bounding_box() const = 0;

    virtual double pdf_value(const Point3& origin, const Vec3& v) const {
        return 0.0;
    }

    virtual Vec3 random(const Point3& origin) const {
        return Vec3(1, 0, 0);
    }
};

class Sphere : public Hittable {
public:
    Sphere(Point3 center, double radius, std::shared_ptr<RTMaterial> mat)
        : center1(center), radius(radius), mat(mat), is_moving(false) {
        Vec3 rvec(radius, radius, radius);
        bbox = AABB(center1 - rvec, center1 + rvec);
    }

    Sphere(Point3 center1, Point3 center2, double radius, std::shared_ptr<RTMaterial> mat)
        : center1(center1), center2(center2), radius(radius), mat(mat), is_moving(true) {
        Vec3 rvec(radius, radius, radius);
        AABB box1(center1 - rvec, center1 + rvec);
        AABB box2(center2 - rvec, center2 + rvec);
        bbox = AABB(box1, box2);
    }

    bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const override {
        Point3 center = is_moving ? sphere_center(r.tm) : center1;
        
        Vec3 oc = r.origin - center;
        auto a = r.direction.length_squared();
        auto half_b = dot(oc, r.direction);
        auto c = oc.length_squared() - radius * radius;

        auto discriminant = half_b * half_b - a * c;
        if (discriminant < 0) return false;
        auto sqrtd = sqrt(discriminant);

        auto root = (-half_b - sqrtd) / a;
        if (!ray_t.surrounds(root)) {
            root = (-half_b + sqrtd) / a;
            if (!ray_t.surrounds(root))
                return false;
        }

        rec.t = root;
        rec.p = r.at(rec.t);
        Vec3 outward_normal = (rec.p - center) / radius;
        rec.set_face_normal(r, outward_normal);

        get_sphere_uv(outward_normal, rec.u, rec.v);

        rec.mat = mat;

        return true;
    }

    AABB bounding_box() const override {
        return bbox;
    }

private:
    Point3 center1;
    Point3 center2;
    double radius;
    std::shared_ptr<RTMaterial> mat;
    bool is_moving;
    AABB bbox;

    Point3 sphere_center(double time) const {
        return center1 + time * (center2 - center1);
    }
    
    static void get_sphere_uv(const Point3& p, double& u, double& v) {
        auto theta = acos(-p.y);
        auto phi = atan2(-p.z, p.x) + pi;

        u = phi / (2 * pi);
        v = theta / pi;
    }
};

class HittableList : public Hittable {
public:
    std::vector<std::shared_ptr<Hittable>> objects;

    HittableList() {}
    HittableList(std::shared_ptr<Hittable> object) { add(object); }

    void clear() { objects.clear(); }

    void add(std::shared_ptr<Hittable> object) {
        objects.push_back(object);
        bbox = AABB(bbox, object->bounding_box());
    }

    bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const override {
        HitRecord temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }

    AABB bounding_box() const override {
        return bbox;
    }

    double pdf_value(const Point3& origin, const Vec3& v) const override {
        auto weight = 1.0 / objects.size();
        auto sum = 0.0;

        for (const auto& object : objects)
            sum += weight * object->pdf_value(origin, v);

        return sum;
    }

private:
    AABB bbox;
};

#endif 