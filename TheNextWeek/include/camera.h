#ifndef RT_CAMERA_H
#define RT_CAMERA_H

#include "rtweekend.h"
#include "ray.h"
#include "vec3.h"
#include <cmath>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

class RTCamera {
public:
    Point3 position;
    Point3 look_at;
    Vec3 vup;
    double vfov;
    double aspect_ratio;
    double aperture;
    double focus_dist;

    double time0 = 0;
    double time1 = 0;

    Point3 origin;
    Vec3 horizontal;
    Vec3 vertical;
    Point3 lower_left_corner;
    Vec3 u, v, w;
    double lens_radius;

    RTCamera(Point3 position = Point3(0, 0, 0),
           Point3 look_at = Point3(0, 0, -1),
           Vec3 vup = Vec3(0, 1, 0),
           double vfov = 90.0,
           double aspect_ratio = 16.0 / 9.0,
           double aperture = 0.0,
           double focus_dist = 1.0,
           double time0 = 0.0,
           double time1 = 0.0)
        : position(position), look_at(look_at), vup(vup), vfov(vfov),
          aspect_ratio(aspect_ratio), aperture(aperture), focus_dist(focus_dist) {
        update();
    }

    void update() {
        double theta = degrees_to_radians(vfov);
        double h = std::tan(theta / 2.0);
        double viewport_height = 2.0 * h;
        double viewport_width = aspect_ratio * viewport_height;

        w = unit_vector(position - look_at);
        u = unit_vector(cross(vup, w));
        v = cross(w, u);

        origin = position;
        horizontal = focus_dist * viewport_width * u;
        vertical = focus_dist * viewport_height * v;
        lower_left_corner = origin - horizontal / 2.0 - vertical / 2.0 - focus_dist * w;

        lens_radius = aperture / 2.0;
    }

    RTRay get_ray(double s, double t) const {
        Vec3 rd = lens_radius * random_in_unit_disk();
        Vec3 offset = u * rd.x + v * rd.y;

        double ray_time = random_double(time0, time1);

        return RTRay(origin + offset,
                   lower_left_corner + s * horizontal + t * vertical - origin - offset, ray_time);
    }

    void move_forward(double speed) {
        Vec3 forward = unit_vector(look_at - position);
        position += forward * speed;
        look_at += forward * speed;
        update();
    }

    void move_right(double speed) {
        position += u * speed;
        look_at += u * speed;
        update();
    }

    void move_up(double speed) {
        position += vup * speed;
        look_at += vup * speed;
        update();
    }

    void rotate(double yaw, double pitch) {
        Vec3 direction = look_at - position;
        double distance = direction.length();

        double current_pitch = std::asin(direction.y / distance);
        double current_yaw = std::atan2(direction.z, direction.x);

        current_yaw += yaw;
        current_pitch += pitch;

        const double max_pitch = pi / 2.0 - 0.01;
        if (current_pitch > max_pitch) current_pitch = max_pitch;
        if (current_pitch < -max_pitch) current_pitch = -max_pitch;

        direction.x = std::cos(current_pitch) * std::cos(current_yaw);
        direction.y = std::sin(current_pitch);
        direction.z = std::cos(current_pitch) * std::sin(current_yaw);

        look_at = position + unit_vector(direction) * distance;
        update();
    }
};

#endif // RT_CAMERA_H