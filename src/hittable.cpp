#include "hittable.h"
#include "bvh_node.h"
#include <cmath>

namespace rt
{
    void HitRecord::setFaceNormal(const Ray &r, const Vec3 &outwardNormal)
    {
        frontFace = dot(r.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }

    static void get_sphere_uv(const Vec3& p, float& u, float& v) {
        float theta = std::acos(-p.y);
        float phi = std::atan2(-p.z, p.x) + M_PI;

        u = phi / (2 * M_PI);
        v = theta / M_PI;
    }

    bool Sphere::hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
    {
        Vec3 oc = r.origin - center;
        float a = r.direction.lengthSq();
        float hb = dot(oc, r.direction);
        float c = oc.lengthSq() - radius * radius;
        float disc = hb * hb - a * c;

        if (disc < 0)
            return false;

        float sqrtD = std::sqrt(disc);
        float root = (-hb - sqrtD) / a;

        if (root < tMin || root > tMax)
        {
            root = (-hb + sqrtD) / a;
            if (root < tMin || root > tMax)
                return false;
        }

        rec.t = root;
        rec.p = r.at(root);
        rec.setFaceNormal(r, (rec.p - center) / radius);

        Vec3 outward_normal = (rec.p - center) / radius;
        rec.setFaceNormal(r, outward_normal);

        get_sphere_uv(outward_normal, rec.u, rec.v);

        rec.mat = mat;
        return true;
    }

    void Quad::init(Vec3 _Q, Vec3 _u, Vec3 _v, Material _mat)
    {
        Q = _Q;
        u = _u;
        v = _v;
        mat = _mat;
        Vec3 n = cross(u, v);
        normal = normalize(n);
        D = dot(normal, Q);
        w = n / dot(n, n);
    }

    bool Quad::hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
    {
        float denom = dot(normal, r.direction);
        if (std::abs(denom) < 1e-8)
            return false;

        float t = (D - dot(normal, r.origin)) / denom;
        if (t < tMin || t > tMax)
            return false;

        Vec3 intersection = r.at(t);
        Vec3 planar_hitpt_vector = intersection - Q;
        float alpha = dot(w, cross(planar_hitpt_vector, v));
        float beta = dot(w, cross(u, planar_hitpt_vector));

        if (alpha < 0 || alpha > 1 || beta < 0 || beta > 1)
            return false;

        rec.t = t;
        rec.p = intersection;
        rec.mat = mat;
        rec.setFaceNormal(r, normal);

        rec.u = alpha;
        rec.v = beta;

        return true;
    }

    bool Triangle::hit(const Ray& r, float tMin, float tMax, HitRecord& rec) const {
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 h = cross(r.direction, edge2);
        float a = dot(edge1, h);
        if (std::abs(a) < 1e-8f) return false;

        float f = 1.0f / a;
        Vec3 s = r.origin - v0;
        float u = f * dot(s, h);
        if (u < 0.0f || u > 1.0f) return false;

        Vec3 q = cross(s, edge1);
        float v = f * dot(r.direction, q);
        if (v < 0.0f || u + v > 1.0f) return false;

        float t = f * dot(edge2, q);
        if (t < tMin || t > tMax) return false;

        rec.t = t;
        rec.p = r.at(t);
        
        Vec3 outwardNormal = normalize(cross(edge1, edge2));
        rec.setFaceNormal(r, outwardNormal);
        rec.mat = mat;
        
        rec.u = u;
        rec.v = v;

        return true;
    }

    Scene::~Scene() {
        delete bvhRoot;
    }

    void Scene::buildBVH()
    {
        delete bvhRoot;
        bvhRoot = nullptr;

        std::vector<BvhItem> items;
        items.reserve(spheres.size() + quads.size() + triangles.size());

        for (const auto &s : spheres)
            items.push_back({s.boundingBox(), &s, nullptr, nullptr, nullptr});
        for (const auto &q : quads)
            items.push_back({q.boundingBox(), nullptr, &q, nullptr, nullptr});
        for (const auto &t : triangles)
            items.push_back({t.boundingBox(), nullptr, nullptr, &t, nullptr});
        for (const auto &m : meshes)
            items.push_back({m.boundingBox(), nullptr, nullptr, nullptr, &m});

        if (!items.empty())
        {
            bvhRoot = new BvhNode(items, 0, items.size());
        }
    }

    bool Scene::hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
    {
        HitRecord tmp;
        bool hitAny = false;
        float best = tMax;

        for (const auto &sphere : spheres)
        {
            if (sphere.hit(r, tMin, best, tmp))
            {
                hitAny = true;
                best = tmp.t;
                rec = tmp;
            }
        }

        for (const auto &quad : quads)
        {
            if (quad.hit(r, tMin, best, tmp))
            {
                hitAny = true;
                best = tmp.t;
                rec = tmp;
            }
        }
        for (const auto &tri : triangles)
        {
            if (tri.hit(r, tMin, best, tmp))
            {
                hitAny = true;
                best = tmp.t;
                rec = tmp;
            }
        }

        for (const auto &mesh : meshes) {
            if (mesh.hit(r, tMin, best, tmp)) { hitAny = true; best = tmp.t; rec = tmp; }
        }
        return hitAny;
    }
}