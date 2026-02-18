#include "hittable.h"
#include <cmath>

namespace rt {

void HitRecord::setFaceNormal(const Ray& r, const Vec3& outwardNormal) {
    frontFace = dot(r.direction, outwardNormal) < 0;
    normal    = frontFace ? outwardNormal : -outwardNormal;
}

bool Sphere::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
    Vec3  oc   = r.origin - center;
    float a    = r.direction.lengthSq();
    float hb   = dot(oc, r.direction);
    float c    = oc.lengthSq() - radius * radius;
    float disc = hb * hb - a * c;
    if (disc < 0) return false;

    float sqrtD = std::sqrt(disc);
    float root  = (-hb - sqrtD) / a;
    if (root < tMin || root > tMax) {
        root = (-hb + sqrtD) / a;
        if (root < tMin || root > tMax) return false;
    }
    rec.t = root;
    rec.p = r.at(root);
    rec.setFaceNormal(r, (rec.p - center) / radius);
    rec.mat = mat;
    return true;
}

bool Scene::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
    HitRecord tmp;
    bool  hitAny = false;
    float best   = tMax;
    for (const auto& sphere : spheres) {
        if (sphere.hit(r, tMin, best, tmp)) {
            hitAny = true;
            best   = tmp.t;
            rec    = tmp;
        }
    }
    return hitAny;
}

} 