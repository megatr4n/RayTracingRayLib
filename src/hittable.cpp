#include "hittable.h"
#include "bvh_node.h"
#include <cmath>

#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include <iostream>

namespace rt
{

    bool loadMeshFromOBJ(const std::string &filename, Mesh &outMesh, Material mat, Vec3 position, float scale)
    {
        tinyobj::ObjReaderConfig reader_config;
        tinyobj::ObjReader reader;

        if (!reader.ParseFromFile(filename, reader_config))
        {
            if (!reader.Error().empty())
            {
                std::cerr << "TinyObjReader: " << reader.Error();
            }
            return false;
        }

        auto &attrib = reader.GetAttrib();
        auto &shapes = reader.GetShapes();

        outMesh.mat = mat;
        outMesh.position = position;
        outMesh.localTriangles.clear();

        for (size_t s = 0; s < shapes.size(); s++)
        {
            size_t index_offset = 0;
            for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
            {
                Vec3 v[3];
                Vec3 n[3];
                bool has_normals = true;

                for (size_t v_idx = 0; v_idx < 3; v_idx++)
                {
                    tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v_idx];
                    v[v_idx] = {
                        attrib.vertices[3 * size_t(idx.vertex_index) + 0] * scale,
                        attrib.vertices[3 * size_t(idx.vertex_index) + 1] * scale,
                        attrib.vertices[3 * size_t(idx.vertex_index) + 2] * scale};

                    if (idx.normal_index >= 0)
                    {
                        n[v_idx] = {
                            attrib.normals[3 * size_t(idx.normal_index) + 0],
                            attrib.normals[3 * size_t(idx.normal_index) + 1],
                            attrib.normals[3 * size_t(idx.normal_index) + 2]};
                    }
                    else
                    {
                        has_normals = false;
                    }
                }

                Triangle tri;
                tri.v0 = v[0];
                tri.v1 = v[1];
                tri.v2 = v[2];

                if (has_normals)
                {
                    tri.n0 = n[0];
                    tri.n1 = n[1];
                    tri.n2 = n[2];
                    tri.hasNormals = true;
                }

                tri.mat = mat;
                outMesh.localTriangles.push_back(tri);

                index_offset += 3;
            }
        }
        return true;
    }

    bool Mesh::hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
    {
        Ray localRay(r.origin - position, r.direction);
        bool hitAny = false;

        if (bvh) {
            if (bvh->hit(localRay, tMin, tMax, rec)) hitAny = true;
        } else {
            float best = tMax;
            HitRecord tmp;
            for (const auto &tri : localTriangles) {
                if (tri.hit(localRay, tMin, best, tmp)) {
                    hitAny = true;
                    best = tmp.t;
                    rec = tmp;
                }
            }
        }

        if (hitAny) {
            rec.p = rec.p + position;
            rec.mat = mat;
        }
        return hitAny;
    }

    void HitRecord::setFaceNormal(const Ray &r, const Vec3 &outwardNormal)
    {
        frontFace = dot(r.direction, outwardNormal) < 0;
        normal = frontFace ? outwardNormal : -outwardNormal;
    }

    static void get_sphere_uv(const Vec3 &p, float &u, float &v)
    {
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

    bool Triangle::hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
    {
        Vec3 edge1 = v1 - v0;
        Vec3 edge2 = v2 - v0;
        Vec3 h = cross(r.direction, edge2);
        float a = dot(edge1, h);
        if (std::abs(a) < 1e-8f)
            return false;

        float f = 1.0f / a;
        Vec3 s = r.origin - v0;
        float u = f * dot(s, h);
        if (u < 0.0f || u > 1.0f)
            return false;

        Vec3 q = cross(s, edge1);
        float v = f * dot(r.direction, q);
        if (v < 0.0f || u + v > 1.0f)
            return false;

        float t = f * dot(edge2, q);
        if (t < tMin || t > tMax)
            return false;

        rec.t = t;
        rec.p = r.at(t);

        Vec3 outwardNormal;
        if (hasNormals)
        {
            float w = 1.0f - u - v;
            outwardNormal = normalize(n0 * w + n1 * u + n2 * v);
        }
        else
        {
            outwardNormal = normalize(cross(edge1, edge2));
        }

        rec.setFaceNormal(r, outwardNormal);
        rec.mat = mat;

        rec.u = u;
        rec.v = v;

        return true;
    }

    Scene::~Scene(){}

    void Scene::buildBVH() {
        bvh = std::make_shared<BvhTree>();

        std::vector<BvhItem> items;
        items.reserve(spheres.size() + quads.size() + triangles.size() + meshes.size());

        for (const auto &s : spheres)
            items.push_back({s.boundingBox(), s.boundingBox().centroid(), &s, nullptr, nullptr, nullptr});
        for (const auto &q : quads)
            items.push_back({q.boundingBox(), q.boundingBox().centroid(), nullptr, &q, nullptr, nullptr});
        for (const auto &t : triangles)
            items.push_back({t.boundingBox(), t.boundingBox().centroid(), nullptr, nullptr, &t, nullptr});
        for (const auto &m : meshes)
            items.push_back({m.boundingBox(), m.boundingBox().centroid(), nullptr, nullptr, nullptr, &m});

        bvh->build(items);
    }

    void Mesh::buildBVH() {
        bvh = std::make_shared<BvhTree>();

        if (localTriangles.empty()) return;

        std::vector<BvhItem> items;
        items.reserve(localTriangles.size());

        for (const auto &tri : localTriangles) {
            items.push_back({tri.boundingBox(), tri.boundingBox().centroid(), nullptr, nullptr, &tri, nullptr});
        }
        bvh->build(items);
    }

    bool Scene::hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const {
        if (bvh) return bvh->hit(r, tMin, tMax, rec);
        return false;
    }
}