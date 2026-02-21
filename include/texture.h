#pragma once
#include "stb_image.h"
#include "vec3.h"
#include <memory>
#include <cmath>

#include <iostream>
#include <algorithm>

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

class ImageTexture : public Texture {
public:
    ImageTexture(const char* filename) {
        int components_per_pixel = 3; 
        data = stbi_load(filename, &width, &height, &bytes_per_pixel, components_per_pixel);
        if (!data) {
            std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
            width = height = 0;
        }
        bytes_per_pixel = components_per_pixel;
    }

    ~ImageTexture() {
        if (data) stbi_image_free(data); 
    }

    Vec3 value(float u, float v, const Vec3& p) const override {
        if (data == nullptr) return Vec3(1.0f, 0.0f, 1.0f);

        u = std::clamp(u, 0.0f, 1.0f);
        v = 1.0f - std::clamp(v, 0.0f, 1.0f); 

        auto i = static_cast<int>(u * width);
        auto j = static_cast<int>(v * height);

        if (i >= width)  i = width - 1;
        if (j >= height) j = height - 1;

        const float color_scale = 1.0f / 255.0f;
        auto pixel = data + j * bytes_per_pixel * width + i * bytes_per_pixel;

        return Vec3(color_scale * pixel[0], color_scale * pixel[1], color_scale * pixel[2]);
    }

private:
    unsigned char* data;
    int width, height;
    int bytes_per_pixel;
};

} 

