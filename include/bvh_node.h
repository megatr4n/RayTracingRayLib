#pragma once
#include "aabb.h"
#include "hittable.h"
#include <vector>
#include <algorithm>

namespace rt
{

    struct BvhItem
    {
        AABB box;
        Vec3 centroid;

        const Sphere *sphere = nullptr;
        const Quad *quad = nullptr;
        const Triangle *tri = nullptr;
        const Mesh *mesh = nullptr;

        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
        {
            if (sphere)
                return sphere->hit(r, tMin, tMax, rec);
            if (quad)
                return quad->hit(r, tMin, tMax, rec);
            if (tri)
                return tri->hit(r, tMin, tMax, rec);
            if (mesh)
                return mesh->hit(r, tMin, tMax, rec);
            return false;
        }
    };

    class BvhNode
    {
    public:
        AABB box;
        BvhNode *left = nullptr;
        BvhNode *right = nullptr;
        std::vector<BvhItem> leafItems;

        BvhNode(std::vector<BvhItem> &objects, size_t start, size_t end)
        {
            for (size_t i = start; i < end; ++i)
            {
                box.extend(objects[i].box);
            }
            int count = end - start;
            if (count <= 2)
            {
                makeLeaf(objects, start, end);
                return;
            }

            constexpr int BINS = 8;
            float bestCost = 1e9f;
            int bestAxis = -1;
            int bestSplit = -1;

            for (int axis = 0; axis < 3; ++axis)
            {
                float minCentroid = 1e9f, maxCentroid = -1e9f;
                for (size_t i = start; i < end; ++i)
                {
                    minCentroid = std::min(minCentroid, objects[i].centroid[axis]);
                    maxCentroid = std::max(maxCentroid, objects[i].centroid[axis]);
                }
                if (minCentroid == maxCentroid)
                    continue;

                struct Bin
                {
                    AABB bounds;
                    int count = 0;
                } bins[BINS];

                float scale = BINS / (maxCentroid - minCentroid);

                for (size_t i = start; i < end; ++i)
                {
                    int binIdx = std::min(BINS - 1, std::max(0, (int)((objects[i].centroid[axis] - minCentroid) * scale)));
                    bins[binIdx].count++;
                    if (bins[binIdx].count == 1)
                        bins[binIdx].bounds = objects[i].box;
                    else
                        bins[binIdx].bounds.extend(objects[i].box);
                }

                float leftArea[BINS - 1];
                int leftCount[BINS - 1];
                AABB leftBox;
                int leftSum = 0;

                for (int i = 0; i < BINS - 1; ++i)
                {
                    leftSum += bins[i].count;
                    leftCount[i] = leftSum;
                    if (bins[i].count > 0)
                    {
                        if (leftSum == bins[i].count)
                            leftBox = bins[i].bounds;
                        else
                            leftBox.extend(bins[i].bounds);
                    }
                    leftArea[i] = leftBox.area();
                }

                AABB rightBox;
                int rightSum = 0;
                for (int i = BINS - 1; i > 0; --i)
                {
                    rightSum += bins[i].count;
                    if (bins[i].count > 0)
                    {
                        if (rightSum == bins[i].count)
                            rightBox = bins[i].bounds;
                        else
                            rightBox.extend(bins[i].bounds);
                    }

                    float cost = leftCount[i - 1] * leftArea[i - 1] + rightSum * rightBox.area();
                    if (cost < bestCost)
                    {
                        bestCost = cost;
                        bestAxis = axis;
                        bestSplit = i;
                    }
                }
            }

            float currentLeafCost = box.area() * count;

            if (bestCost >= currentLeafCost || bestAxis == -1)
            {
                makeLeaf(objects, start, end);
                return;
            }

            float minCentroid = 1e9f, maxCentroid = -1e9f;
            for (size_t i = start; i < end; ++i)
            {
                minCentroid = std::min(minCentroid, objects[i].centroid[bestAxis]);
                maxCentroid = std::max(maxCentroid, objects[i].centroid[bestAxis]);
            }
            float scale = BINS / (maxCentroid - minCentroid);

            auto midIter = std::partition(objects.begin() + start, objects.begin() + end,
                                          [=](const BvhItem &a)
                                          {
                                              int binIdx = std::min(BINS - 1, std::max(0, (int)((a.centroid[bestAxis] - minCentroid) * scale)));
                                              return binIdx < bestSplit;
                                          });

            size_t mid = std::distance(objects.begin(), midIter);
            if (mid == start || mid == end)
            {
                mid = start + count / 2;
            }

            left = new BvhNode(objects, start, mid);
            right = new BvhNode(objects, mid, end);
        }

        void makeLeaf(std::vector<BvhItem> &objects, size_t start, size_t end)
        {
            for (size_t i = start; i < end; ++i)
            {
                leafItems.push_back(objects[i]);
            }
        }

        ~BvhNode()
        {
            delete left;
            delete right;
        }

        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
        {
            if (!box.hit(r, tMin, tMax))
                return false;

            bool hitAny = false;
            float closest = tMax;

            if (!leafItems.empty())
            {
                for (const auto &item : leafItems)
                {
                    if (item.hit(r, tMin, closest, rec))
                    {
                        hitAny = true;
                        closest = rec.t;
                    }
                }
                return hitAny;
            }

            bool hitLeft = left && left->hit(r, tMin, closest, rec);
            if (hitLeft)
                closest = rec.t;
            bool hitRight = right && right->hit(r, tMin, closest, rec);

            return hitLeft || hitRight;
        }
    };
    struct LinearBvhNode
    {
        AABB box;
        int rightOffset;
        int itemsOffset; 
        int itemsCount; 
    };

    inline int flattenBvhTree(BvhNode *node, std::vector<LinearBvhNode> &flatNodes, std::vector<BvhItem> &flatItems)
    {
        if (!node)
            return -1;

        int nodeIndex = flatNodes.size();
        flatNodes.push_back(LinearBvhNode());
        flatNodes[nodeIndex].box = node->box;

        if (!node->leafItems.empty())
        {
            flatNodes[nodeIndex].itemsOffset = flatItems.size();
            flatNodes[nodeIndex].itemsCount = node->leafItems.size();
            flatNodes[nodeIndex].rightOffset = 0;
            for (const auto &item : node->leafItems)
            {
                flatItems.push_back(item);
            }
        }
        else
        {
            flatNodes[nodeIndex].itemsCount = 0;
            flattenBvhTree(node->left, flatNodes, flatItems);
            flatNodes[nodeIndex].rightOffset = flattenBvhTree(node->right, flatNodes, flatItems);
        }
        return nodeIndex;
    }

    class BvhTree
    {
    public:
        std::vector<LinearBvhNode> flatNodes;
        std::vector<BvhItem> flatItems;

        void build(std::vector<BvhItem> &items)
        {
            if (items.empty())
                return;

            BvhNode *root = new BvhNode(items, 0, items.size());

            flatNodes.clear();
            flatItems.clear();
            flattenBvhTree(root, flatNodes, flatItems);

            delete root;
        }

        bool hit(const Ray &r, float tMin, float tMax, HitRecord &rec) const
        {
            if (flatNodes.empty())
                return false;

            bool hitAny = false;
            float closest = tMax;

            int stack[64];
            int stackPtr = 0;
            stack[stackPtr++] = 0;

            while (stackPtr > 0)
            {
                int nodeIdx = stack[--stackPtr];
                const LinearBvhNode &node = flatNodes[nodeIdx];

                if (node.box.hit(r, tMin, closest))
                {
                    if (node.itemsCount > 0)
                    {
                        for (int i = 0; i < node.itemsCount; ++i)
                        {
                            if (flatItems[node.itemsOffset + i].hit(r, tMin, closest, rec))
                            {
                                hitAny = true;
                                closest = rec.t;
                            }
                        }
                    }
                    else
                    {
                        stack[stackPtr++] = node.rightOffset;
                        stack[stackPtr++] = nodeIdx + 1;
                    }
                }
            }
            return hitAny;
        }
    };

}