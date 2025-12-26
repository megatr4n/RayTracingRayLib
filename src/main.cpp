#include "raylib.h"
#include "../include/vec3.h"
#include "../include/ray.h"
#include "../include/camera.h"
#include "../include/hittable.h"
#include "../include/material.h"
#include <memory>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

bool Lambertian::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    (void)r_in;
    Vec3 scatter_direction = rec.normal + random_unit_vector();
    if (scatter_direction.near_zero())
        scatter_direction = rec.normal;
    scattered = RTRay(rec.p, scatter_direction);
    attenuation = albedo;
    return true;
}

bool Metal::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    Vec3 reflected = reflect(unit_vector(r_in.direction), rec.normal);
    scattered = RTRay(rec.p, reflected + fuzz * random_in_unit_sphere());
    attenuation = albedo;
    return (dot(scattered.direction, rec.normal) > 0);
}

bool Dielectric::scatter(const RTRay& r_in, const HitRecord& rec, Color3& attenuation, RTRay& scattered) const {
    attenuation = Color3(1.0, 1.0, 1.0);
    double refraction_ratio = rec.front_face ? (1.0 / ir) : ir;

    Vec3 unit_direction = unit_vector(r_in.direction);
    double cos_theta = fmin(dot(-unit_direction, rec.normal), 1.0);
    double sin_theta = sqrt(1.0 - cos_theta * cos_theta);

    bool cannot_refract = refraction_ratio * sin_theta > 1.0;
    Vec3 direction;

    if (cannot_refract || reflectance(cos_theta, refraction_ratio) > ((double)rand() / RAND_MAX))
        direction = reflect(unit_direction, rec.normal);
    else
        direction = refract(unit_direction, rec.normal, refraction_ratio);

    scattered = RTRay(rec.p, direction);
    return true;
}

    Color3 ray_color(const RTRay& r, const HittableList& world, int depth) {
    if (depth <= 0)
        return Color3(0, 0, 0);

    HitRecord rec;
    if (world.hit(r, 0.001, INFINITY, rec)) {
        RTRay scattered;
        Color3 attenuation;
        if (rec.mat->scatter(r, rec, attenuation, scattered))
            return attenuation * ray_color(scattered, world, depth - 1);
        return Color3(0, 0, 0);
    }

    Vec3 unit_direction = unit_vector(r.direction);
    double t = 0.5 * (unit_direction.y + 1.0);
    return (1.0 - t) * Color3(1.0, 1.0, 1.0) + t * Color3(0.5, 0.7, 1.0);
}

    HittableList create_scene() {
        HittableList world;

        auto ground_material = std::make_shared<Lambertian>(Color3(0.5, 0.5, 0.5));
        world.add(std::make_shared<Sphere>(Point3(0, -100.5, -1), 100, ground_material));

        auto material_center = std::make_shared<Lambertian>(Color3(0.7, 0.3, 0.3));
        world.add(std::make_shared<Sphere>(Point3(0, 0, -1), 0.5, material_center));

        auto material_left = std::make_shared<Dielectric>(1.5);
        world.add(std::make_shared<Sphere>(Point3(-1, 0, -1), 0.5, material_left));

        auto material_right = std::make_shared<Metal>(Color3(0.8, 0.6, 0.2), 0.0);
        world.add(std::make_shared<Sphere>(Point3(1, 0, -1), 0.5, material_right));

        return world;
}

int main() {
    srand(static_cast<unsigned int>(time(NULL)));

    const int screen_width = 800;
    const int screen_height = 450;
    const double aspect_ratio = (double)screen_width / screen_height;

    InitWindow(screen_width, screen_height, "Ray Tracer (Raylib) - Book 1");
    SetTargetFPS(60);

    int samples_per_pixel = 1;
    int max_depth = 10;
    bool is_rendering = true;
    int accumulated_samples = 0;

    Point3 lookfrom(3, 1, 2);
    Point3 lookat(0, 0, -1);
    Vec3 vup(0, 1, 0);
    double dist_to_focus = 3.0;
    double aperture = 0.1;

    RTCamera camera(lookfrom, lookat, vup, 40.0, aspect_ratio, aperture, dist_to_focus);

    std::vector<Color3> accumulation_buffer(screen_width * screen_height, Color3(0, 0, 0));
    Image render_image = GenImageColor(screen_width, screen_height, BLACK);
    Texture2D render_texture = LoadTextureFromImage(render_image);

    HittableList world = create_scene();

    float move_speed = 0.05f;
    float mouse_sensitivity = 0.003f;
    bool camera_moved = false;
    Vector2 last_mouse_pos = GetMousePosition();

    DisableCursor();

    for (int j = 0; j < screen_height; j++) {
        for (int i = 0; i < screen_width; i++) {
            Color3 pixel_color(0, 0, 0);
            for (int s = 0; s < 1; s++) {
                double u = (i + 0.5) / (screen_width - 1);
                double v = (j + 0.5) / (screen_height - 1);
                RTRay r = camera.get_ray(u, 1.0 - v);
                pixel_color += ray_color(r, world, max_depth);
            }
            accumulation_buffer[j * screen_width + i] = pixel_color;
            
            Color3 avg_color = pixel_color;
            avg_color.x = sqrt(avg_color.x);
            avg_color.y = sqrt(avg_color.y);
            avg_color.z = sqrt(avg_color.z);
            
            unsigned char r = (unsigned char)(256 * fmax(0.0, fmin(0.999, avg_color.x)));
            unsigned char g = (unsigned char)(256 * fmax(0.0, fmin(0.999, avg_color.y)));
            unsigned char b = (unsigned char)(256 * fmax(0.0, fmin(0.999, avg_color.z)));
            ImageDrawPixel(&render_image, i, j, {r, g, b, 255});
        }
    }
    UpdateTexture(render_texture, render_image.data);
    accumulated_samples = 1;
}
    

    
