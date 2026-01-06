#pragma once
#include <cmath>
#include <iostream>

class Vec3 {
public:
    double x, y, z;

    double operator[](int i) const {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    double& operator[](int i) {
        if (i == 0) return x;
        if (i == 1) return y;
        return z;
    }

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double x, double y, double z) : x(x), y(y), z(z) {}

    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    Vec3 operator+(const Vec3& v) const { return Vec3(x + v.x, y + v.y, z + v.z); }
    Vec3 operator-(const Vec3& v) const { return Vec3(x - v.x, y - v.y, z - v.z); }
    Vec3 operator*(const Vec3& v) const { return Vec3(x * v.x, y * v.y, z * v.z); }
    Vec3 operator*(double t) const { return Vec3(x * t, y * t, z * t); }
    Vec3 operator/(double t) const { return *this * (1.0 / t); }

    Vec3& operator+=(const Vec3& v) {
        x += v.x; y += v.y; z += v.z;
        return *this;
    }

    Vec3& operator*=(double t) {
        x *= t; y *= t; z *= t;
        return *this;
    }

    double length() const {
        return std::sqrt(length_squared());
    }

    double length_squared() const {
        return x * x + y * y + z * z;
    }

    bool near_zero() const {
        const double s = 1e-8;
        return (std::fabs(x) < s) && (std::fabs(y) < s) && (std::fabs(z) < s);
    }

    static Vec3 random() {
        return Vec3(random_double(), random_double(), random_double());
    }

    static Vec3 random(double min, double max) {
        return Vec3(random_double(min, max), random_double(min, max), random_double(min, max));
    }

private:
    static double random_double() {
        return rand() / (RAND_MAX + 1.0);
    }

    static double random_double(double min, double max) {
        return min + (max - min) * random_double();
    }
};

using Point3 = Vec3;
using Color3 = Vec3;  

inline Vec3 operator*(double t, const Vec3& v) {
    return v * t;
}

inline double dot(const Vec3& u, const Vec3& v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

inline Vec3 cross(const Vec3& u, const Vec3& v) {
    return Vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

inline Vec3 unit_vector(Vec3 v) {
    return v / v.length();
}

inline Vec3 random_in_unit_sphere() {
    while (true) {
        Vec3 p = Vec3::random(-1, 1);
        if (p.length_squared() < 1)
            return p;
    }
}

inline Vec3 random_unit_vector() {
    return unit_vector(random_in_unit_sphere());
}

inline Vec3 random_in_hemisphere(const Vec3& normal) {
    Vec3 in_unit_sphere = random_in_unit_sphere();
    if (dot(in_unit_sphere, normal) > 0.0)
        return in_unit_sphere;
    else
        return -in_unit_sphere;
}

inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2 * dot(v, n) * n;
}

inline Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat) {
    double cos_theta = std::fmin(dot(-uv, n), 1.0);
    Vec3 r_out_perp = etai_over_etat * (uv + cos_theta * n);
    Vec3 r_out_parallel = -std::sqrt(std::fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

inline Vec3 random_in_unit_disk() {
    while (true) {
        Vec3 p = Vec3(Vec3::random(-1, 1).x, Vec3::random(-1, 1).y, 0);
        if (p.length_squared() < 1)
            return p;
    }
}