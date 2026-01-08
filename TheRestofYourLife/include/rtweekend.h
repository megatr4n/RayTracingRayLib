#ifndef RTWEEKEND_H
#define RTWEEKEND_H




#include <cmath>
#include <cstdlib>
#include <iostream>
#include <limits>
#include <memory>



using std::make_shared;
using std::shared_ptr;



const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;


inline double degrees_to_radians(double degrees) {
    return degrees * pi / 180.0;
}

inline double random_double() {
    return std::rand() / (RAND_MAX + 1.0);
}

inline double random_double(double min, double max) {
    return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
    return int(random_double(min, max+1));
}


#include "interval.h"
#include "ray.h"
#include "vec3.h"


inline Vec3 random_cosine_direction() {
    auto r1 = random_double();
    auto r2 = random_double();
    auto z = sqrt(1 - r2);

    auto phi = 2 * pi * r1;
    auto x = cos(phi) * sqrt(r2);
    auto y = sin(phi) * sqrt(r2);

    return Vec3(x, y, z);
}

#endif