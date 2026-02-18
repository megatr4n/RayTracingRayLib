#pragma once
#include <cmath>
#include <raylib.h>

namespace rt {

struct Vec3 {
    float x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    Vec3 operator+(const Vec3& o) const { 
        return {x+o.x, y+o.y, z+o.z}; 
    }
    Vec3 operator-(const Vec3& o) const { 
        return {x-o.x, y-o.y, z-o.z}; 
    }
    Vec3 operator*(float t) const { 
        return {x*t, y*t, z*t}; 
    }
    Vec3 operator*(const Vec3& o) const { 
        return {x*o.x, y*o.y, z*o.z}; 
    }
    Vec3 operator/(float t) const { 
        return {x/t, y/t, z/t}; 
    }
    Vec3 operator-() const { 
        return {-x, -y, -z}; 
    }

    Vec3& operator+=(const Vec3& o) { 
        x+=o.x; y+=o.y; z+=o.z; 
        return *this; 
    }
    Vec3& operator*=(float t) { 
        x*=t; y*=t; z*=t; 
        return *this; 
    }
    Vec3& operator-=(const Vec3& o) { 
        x-=o.x; y-=o.y; z-=o.z; 
        return *this; 
    }
    float lengthSq() const { 
        return x*x + y*y + z*z; 
    }
    float length()   const { 
        return std::sqrt(lengthSq()); 
    }

    bool nearZero() const {
        const float s = 1e-8f;
        return std::fabs(x)<s && std::fabs(y)<s && std::fabs(z)<s;
    }

    Color toColor(int samples) const {
        float scale = 1.0f / samples;
        float r = std::sqrt(x * scale);
        float g = std::sqrt(y * scale);
        float b = std::sqrt(z * scale);
        auto clamp01 = [](float v){ return v<0?0.f:v>1?1.f:v; };
        return {
            (unsigned char)(255.99f * clamp01(r)),
            (unsigned char)(255.99f * clamp01(g)),
            (unsigned char)(255.99f * clamp01(b)),
            255
        };
    }
};

inline Vec3 operator*(float t, const Vec3& v) { return v * t; }

inline float dot(const Vec3& a, const Vec3& b) {
    return a.x*b.x + a.y*b.y + a.z*b.z;
}

inline Vec3 cross(const Vec3& a, const Vec3& b) {
    return {
        a.y*b.z - a.z*b.y,
        a.z*b.x - a.x*b.z,
        a.x*b.y - a.y*b.x
    };
}

inline Vec3 normalize(const Vec3& v) { return v / v.length(); }

inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2.0f * dot(v, n) * n;
}

inline Vec3 refract(const Vec3& uv, const Vec3& n, float etaiOverEtat) {
    float cosTheta = std::fmin(dot(-uv, n), 1.0f);
    Vec3 rOutPerp  = etaiOverEtat * (uv + cosTheta * n);
    Vec3 rOutPar   = -std::sqrt(std::fabs(1.0f - rOutPerp.lengthSq())) * n;
    return rOutPerp + rOutPar;
}
}