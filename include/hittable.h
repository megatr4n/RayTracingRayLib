#pragma once
#include "ray.h"
#include "material.h"
#include "aabb.h"
#include <vector>
#include <algorithm>
#include <memory>

namespace rt
{
    class BvhTree;

    struct HitRecord
    {
        Vec3 p;
        Vec3 normal;
        uint32_t matIndex = 0;
        float t = 0;
        float u;
        float v;
        bool frontFace = true;

        void setFaceNormal(const Ray &r, const Vec3 &outwardNormal);
    };

    struct Sphere
    {
        Vec3 center;
        float radius = 0.5f;
        uint32_t matIndex = 0;

        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const;

        AABB boundingBox() const
        {
            Vec3 rVec(radius, radius, radius);
            return AABB(center - rVec, center + rVec);
        }
    };

    struct Quad
    {
        Vec3 Q;
        Vec3 u;
        Vec3 v;
        uint32_t matIndex = 0;

        Vec3 normal;
        float D;
        Vec3 w;

        void init(Vec3 _Q, Vec3 _u, Vec3 _v, uint32_t _matIndex);
        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const;

        AABB boundingBox() const
        {
            Vec3 p0 = Q;
            Vec3 p1 = Q + u;
            Vec3 p2 = Q + v;
            Vec3 p3 = Q + u + v;

            Vec3 minPoint(
                std::min({p0.x, p1.x, p2.x, p3.x}),
                std::min({p0.y, p1.y, p2.y, p3.y}),
                std::min({p0.z, p1.z, p2.z, p3.z}));
            Vec3 maxPoint(
                std::max({p0.x, p1.x, p2.x, p3.x}),
                std::max({p0.y, p1.y, p2.y, p3.y}),
                std::max({p0.z, p1.z, p2.z, p3.z}));

            float pad = 0.0001f;
            if (std::abs(maxPoint.x - minPoint.x) < pad)
            {
                minPoint.x -= pad;
                maxPoint.x += pad;
            }
            if (std::abs(maxPoint.y - minPoint.y) < pad)
            {
                minPoint.y -= pad;
                maxPoint.y += pad;
            }
            if (std::abs(maxPoint.z - minPoint.z) < pad)
            {
                minPoint.z -= pad;
                maxPoint.z += pad;
            }

            return AABB(minPoint, maxPoint);
        }
    };

    struct Triangle
    {
        Vec3 v0, v1, v2;
        Vec3 n0, n1, n2;
        bool hasNormals = false;
        uint32_t matIndex = 0;

        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const;

        AABB boundingBox() const
        {
            Vec3 minPoint(
                std::min({v0.x, v1.x, v2.x}),
                std::min({v0.y, v1.y, v2.y}),
                std::min({v0.z, v1.z, v2.z}));
            Vec3 maxPoint(
                std::max({v0.x, v1.x, v2.x}),
                std::max({v0.y, v1.y, v2.y}),
                std::max({v0.z, v1.z, v2.z}));

            float pad = 0.0001f;
            if (std::abs(maxPoint.x - minPoint.x) < pad)
            {
                minPoint.x -= pad;
                maxPoint.x += pad;
            }
            if (std::abs(maxPoint.y - minPoint.y) < pad)
            {
                minPoint.y -= pad;
                maxPoint.y += pad;
            }
            if (std::abs(maxPoint.z - minPoint.z) < pad)
            {
                minPoint.z -= pad;
                maxPoint.z += pad;
            }

            return AABB(minPoint, maxPoint);
        }
    };

    struct Mesh
    {
        Vec3 position;
        std::vector<Triangle> localTriangles;
        uint32_t matIndex = 0;
        std::shared_ptr<BvhTree> bvh;

        void buildBVH();

        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const;

        AABB boundingBox() const
        {
            if (localTriangles.empty())
                return AABB(position, position);

            Vec3 minPoint = localTriangles[0].v0;
            Vec3 maxPoint = localTriangles[0].v0;

            for (const auto &tri : localTriangles)
            {
                for (const auto &v : {tri.v0, tri.v1, tri.v2})
                {
                    minPoint.x = std::min(minPoint.x, v.x);
                    minPoint.y = std::min(minPoint.y, v.y);
                    minPoint.z = std::min(minPoint.z, v.z);

                    maxPoint.x = std::max(maxPoint.x, v.x);
                    maxPoint.y = std::max(maxPoint.y, v.y);
                    maxPoint.z = std::max(maxPoint.z, v.z);
                }
            }

            minPoint = minPoint + position;
            maxPoint = maxPoint + position;

            float pad = 0.0001f;
            if (std::abs(maxPoint.x - minPoint.x) < pad)
            {
                minPoint.x -= pad;
                maxPoint.x += pad;
            }
            if (std::abs(maxPoint.y - minPoint.y) < pad)
            {
                minPoint.y -= pad;
                maxPoint.y += pad;
            }
            if (std::abs(maxPoint.z - minPoint.z) < pad)
            {
                minPoint.z -= pad;
                maxPoint.z += pad;
            }

            return AABB(minPoint, maxPoint);
        }
    };

    struct Scene
    {
        std::vector<Material> materials;
        std::vector<Sphere> spheres;
        std::vector<Quad> quads;
        std::vector<Triangle> triangles;
        std::vector<Mesh> meshes;
        std::shared_ptr<BvhTree> bvh;

        float lastBvhBuildTimeMs = 0.0f;

        ~Scene();
        void buildBVH();
        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const;
    };
    bool loadMeshFromOBJ(const std::string &filename, Mesh &outMesh, uint32_t matIndex, Vec3 position = {0, 0, 0}, float scale = 1.0f);

}