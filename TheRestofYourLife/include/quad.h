#ifndef QUAD_H
#define QUAD_H

#include "rtweekend.h"
#include "hittable.h"
#include <cmath>

class Quad : public Hittable {
public:
    Quad(const Point3& Q, const Vec3& u, const Vec3& v, std::shared_ptr<RTMaterial> mat): Q(Q), u(u), v(v), mat(mat){
        auto n = cross(u, v);
        normal = unit_vector(n);
        D = dot(normal, Q);
        w = n / dot(n, n);

        set_bounding_box();
    }

    virtual void set_bounding_box() {
        auto bbox_diagonal1 = AABB(Q, Q + u + v);
        auto bbox_diagonal2 = AABB(Q + u, Q + v);
        bbox = AABB(bbox_diagonal1, bbox_diagonal2);
    }

    AABB bounding_box() const override {
        return bbox;
    }

    bool hit(const RTRay& r, interval ray_t, HitRecord& rec) const override {
        auto denom = dot(normal, r.direction);

        if (fabs(denom) < 1e-8)
            return false;

        auto t = (D - dot(normal, r.origin)) / denom;
        if (!ray_t.contains(t))
            return false;

        auto intersection = r.at(t);
        Vec3 planar_hitpt_vector = intersection - Q;
        
        auto alpha = dot(w, cross(planar_hitpt_vector, v));
        auto beta = dot(w, cross(u, planar_hitpt_vector));

        if (!is_interior(alpha, beta, rec))
            return false;

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.set_face_normal(r, normal);

        return true;
    }

    virtual bool is_interior(double a, double b, HitRecord& rec) const {
        interval unit_interval = interval(0, 1);
        if (!unit_interval.contains(a) || !unit_interval.contains(b))
            return false;

        rec.u = a;
        rec.v = b;
        return true;
    }

private:
    Point3 Q;
    Vec3 u, v;
    std::shared_ptr<RTMaterial> mat;
    AABB bbox;
    Vec3 normal;
    double D;
    Vec3 w;
};

#endif