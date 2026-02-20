#pragma once
#include "vec3.h"
#include <cstdlib>
#include <random>

namespace rt {

inline float randomFloat() {
    thread_local std::mt19937 generator(std::random_device{}());
    std::uniform_real_distribution<float> distribution(0.0f, 1.0f);
    return distribution(generator);
}

inline float randomFloat(float min, float max) {
    return min + (max - min) * randomFloat();
}

inline Vec3 randomVec3() {
    return {randomFloat(), randomFloat(), randomFloat()};
}

inline Vec3 randomVec3(float min, float max) {
    return {randomFloat(min,max), randomFloat(min,max), randomFloat(min,max)};
}

inline Vec3 randomInUnitSphere() {
    while (true) {
        Vec3 p = randomVec3(-1, 1);
        if (p.lengthSq() < 1.0f) return p;
    }
}

inline Vec3 randomUnitVector() {
    return normalize(randomInUnitSphere());
}

inline Vec3 randomInUnitDisk() {
    while (true) {
        Vec3 p = {randomFloat(-1,1), randomFloat(-1,1), 0};
        if (p.lengthSq() < 1.0f) return p;
    }
}
}