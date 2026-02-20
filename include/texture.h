#pragma once
#include "vec3.h"
#include <memory>
#include <cmath>

namespace rt {

class Texture {
public:
    virtual ~Texture() = default;
    
    virtual Vec3 value(float u, float v, const Vec3& p) const = 0;
};

class SolidColor : public Texture {
public:
    SolidColor(Vec3 c) : colorValue(c) {}
    SolidColor(float red, float green, float blue) : SolidColor(Vec3(red, green, blue)) {}

    Vec3 value(float u, float v, const Vec3& p) const override {
        return colorValue;
    }

private:
    Vec3 colorValue;
};

class CheckerTexture : public Texture {
public:
    CheckerTexture(float _scale, std::shared_ptr<Texture> _even, std::shared_ptr<Texture> _odd)
        : invScale(1.0f / _scale), even(_even), odd(_odd) {}

    CheckerTexture(float _scale, Vec3 c1, Vec3 c2)
        : invScale(1.0f / _scale), even(std::make_shared<SolidColor>(c1)), odd(std::make_shared<SolidColor>(c2)) {}

    Vec3 value(float u, float v, const Vec3& p) const override {
        auto xInteger = static_cast<int>(std::floor(invScale * p.x));
        auto yInteger = static_cast<int>(std::floor(invScale * p.y));
        auto zInteger = static_cast<int>(std::floor(invScale * p.z));

        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;
        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    float invScale;
    std::shared_ptr<Texture> even;
    std::shared_ptr<Texture> odd;
};

} // namespace rt