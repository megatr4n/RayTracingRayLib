#ifndef RT_TEXTURE_H
#define RT_TEXTURE_H

#include "raylib.h"

#include "rtweekend.h"
#include "vec3.h"

class RTTexture {
public:
    virtual ~RTTexture() = default;
    virtual Color3 value(double u, double v, const Point3& p) const = 0;
};

class SolidColor : public RTTexture {
public:
    SolidColor(const Color3& albedo) : albedo(albedo) {}
    SolidColor(double red, double green, double blue) : SolidColor(Color3(red, green, blue)) {}

    Color3 value(double u, double v, const Point3& p) const override {
        (void)u; (void)v; (void)p;
        return albedo;
    }

private:
    Color3 albedo;
};

class CheckerTexture : public RTTexture {
public:
    CheckerTexture(double scale, std::shared_ptr<RTTexture> even, std::shared_ptr<RTTexture> odd)
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
    std::shared_ptr<RTTexture> even;
    std::shared_ptr<RTTexture> odd;
};

class ImageTexture : public RTTexture {
  public:
      ImageTexture(const char* filename) {
          image = LoadImage(filename);
          
          if (image.data == nullptr) {
              std::cerr << "ERROR: Could not load texture image file '" << filename << "'.\n";
              width = height = 0;
          } else {
              width = image.width;
              height = image.height;
          }
      }
      ~ImageTexture() {
          if (image.data != nullptr) {
              UnloadImage(image);
          }
      }
  
      Color3 value(double u, double v, const Point3& p) const override {
          if (image.height <= 0) return Color3(0, 1, 1);
          u = interval(0, 1).clamp(u);
          v = 1.0 - interval(0, 1).clamp(v); 
  
          auto i = static_cast<int>(u * width);
          auto j = static_cast<int>(v * height);
  
          if (i >= width)  i = width - 1;
          if (j >= height) j = height - 1;
  
          ::Color pixel = GetImageColor(image, i, j);
  
          double scale = 1.0 / 255.0;
          return Color3(scale * pixel.r, scale * pixel.g, scale * pixel.b);
      }
  
  private:
      Image image;
      int width, height;
  };

#endif