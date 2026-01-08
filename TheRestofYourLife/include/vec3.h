#ifndef VEC3_H
#define VEC3_H

#include <cmath>
#include <iostream>
#include <cstdlib>

using std::sqrt;

class Vec3 {
public:
    double x, y, z;

    Vec3() : x(0), y(0), z(0) {}
    Vec3(double e0, double e1, double e2) : x(e0), y(e1), z(e2) {}

    Vec3 operator-() const { return Vec3(-x, -y, -z); }
    double operator[](int i) const { return (i==0) ? x : ((i==1) ? y : z); }
    double& operator[](int i) { return (i==0) ? x : ((i==1) ? y : z); }

    Vec3& operator+=(const Vec3 &v) {
        x += v.x;
        y += v.y;
        z += v.z;
        return *this;
    }

    Vec3& operator*=(const double t) {
        x *= t;
        y *= t;
        z *= t;
        return *this;
    }

    Vec3& operator/=(const double t) {
        return *this *= 1/t;
    }

    double length() const {
        return sqrt(length_squared());
    }

    double length_squared() const {
        return x*x + y*y + z*z;
    }

    static Vec3 random() {
        return Vec3(std::rand() / (RAND_MAX + 1.0), std::rand() / (RAND_MAX + 1.0), std::rand() / (RAND_MAX + 1.0));
    }

    static Vec3 random(double min, double max) {
        return Vec3(min + (max-min)*(std::rand() / (RAND_MAX + 1.0)),
                    min + (max-min)*(std::rand() / (RAND_MAX + 1.0)),
                    min + (max-min)*(std::rand() / (RAND_MAX + 1.0)));
    }
};

using Point3 = Vec3;
using Color3 = Vec3;

inline std::ostream& operator<<(std::ostream &out, const Vec3 &v) {
    return out << v.x << ' ' << v.y << ' ' << v.z;
}

inline Vec3 operator+(const Vec3 &u, const Vec3 &v) {
    return Vec3(u.x + v.x, u.y + v.y, u.z + v.z);
}

inline Vec3 operator-(const Vec3 &u, const Vec3 &v) {
    return Vec3(u.x - v.x, u.y - v.y, u.z - v.z);
}

inline Vec3 operator*(const Vec3 &u, const Vec3 &v) {
    return Vec3(u.x * v.x, u.y * v.y, u.z * v.z);
}

inline Vec3 operator*(double t, const Vec3 &v) {
    return Vec3(t*v.x, t*v.y, t*v.z);
}

inline Vec3 operator*(const Vec3 &v, double t) {
    return t * v;
}

inline Vec3 operator/(Vec3 v, double t) {
    return (1/t) * v;
}

inline double dot(const Vec3 &u, const Vec3 &v) {
    return u.x * v.x + u.y * v.y + u.z * v.z;
}

inline Vec3 cross(const Vec3 &u, const Vec3 &v) {
    return Vec3(u.y * v.z - u.z * v.y,
                u.z * v.x - u.x * v.z,
                u.x * v.y - u.y * v.x);
}

inline Vec3 unit_vector(Vec3 v) {
    return v / v.length();
}

inline Vec3 random_unit_vector() {
    while (true) {
        auto p = Vec3::random(-1,1);
        auto lensq = p.length_squared();
        if (1e-160 < lensq && lensq <= 1)
            return p / sqrt(lensq);
    }
}
inline Vec3 random_in_unit_disk() {
    while (true) {
        auto p = Vec3(Vec3::random(-1,1).x, Vec3::random(-1,1).y, 0);
        if (p.length_squared() < 1)
            return p;
    }
}

inline Vec3 reflect(const Vec3& v, const Vec3& n) {
    return v - 2*dot(v,n)*n;
}

inline Vec3 refract(const Vec3& uv, const Vec3& n, double etai_over_etat) {
    auto cos_theta = fmin(dot(-uv, n), 1.0);
    Vec3 r_out_perp =  etai_over_etat * (uv + cos_theta*n);
    Vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
    return r_out_perp + r_out_parallel;
}

#endif