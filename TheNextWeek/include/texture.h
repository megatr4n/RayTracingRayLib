#ifndef RT_TEXTURE_H
#define RT_TEXTURE_H

#include "rtweekend.h"
#include "vec3.h"

class Texture {
public:
    virtual ~Texture() = default;
    virtual Color3 value(double u, double v, const Point3& p) const = 0;
};

class SolidColor : public Texture {
public:
    SolidColor(const Color3& albedo) : albedo(albedo) {}
    SolidColor(double red, double green, double blue) : SolidColor(Color3(red, green, blue)) {}

    Color3 value(double u, double v, const Point3& p) const override {
        return albedo;
    }

private:
    Color3 albedo;
};

class CheckerTexture : public Texture {
public:
    CheckerTexture(double scale, std::shared_ptr<Texture> even, std::shared_ptr<Texture> odd)
        : inv_scale(1.0 / scale), even(even), odd(odd) {}

    CheckerTexture(double scale, const Color3& c1, const Color3& c2)
        : inv_scale(1.0 / scale),
          even(std::make_shared<SolidColor>(c1)),
          odd(std::make_shared<SolidColor>(c2)) {}

    Color3 value(double u, double v, const Point3& p) const override {
        auto xInteger = static_cast<int>(std::floor(inv_scale * p.x));
        auto yInteger = static_cast<int>(std::floor(inv_scale * p.y));
        auto zInteger = static_cast<int>(std::floor(inv_scale * p.z));

        bool isEven = (xInteger + yInteger + zInteger) % 2 == 0;

        return isEven ? even->value(u, v, p) : odd->value(u, v, p);
    }

private:
    double inv_scale;
    std::shared_ptr<Texture> even;
    std::shared_ptr<Texture> odd;
};

#endif