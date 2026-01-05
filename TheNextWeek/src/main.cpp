#include "raylib.h"

#include "../include/interval.h"
#include "../include/rtweekend.h" 
#include "../include/vec3.h"
#include "../include/ray.h"
#include "../include/camera.h"
#include "../include/hittable.h"
#include "../include/material.h"
#include "../include/bvh.h"
#include "../include/quad.h"
#include "../include/transform.h"
#include "../include/constant_medium.h"

#include <memory>
#include <vector>
#include <cmath>
#include <cstdlib>
#include <ctime>

Color3 ray_color(const RTRay& r, const HittableList& world, int depth) {
    if (depth <= 0)
        return Color3(0, 0, 0);

    HitRecord rec;

        if (!world.hit(r, interval(0.001, infinity), rec))
            return Color3(0, 0, 0);

        Color3 emission_color = rec.mat->emitted(rec.u, rec.v, rec.p);
        RTRay scattered;
        Color3 attenuation;

        if (rec.mat->scatter(r, rec, attenuation, scattered)) {
            return emission_color + attenuation * ray_color(scattered, world, depth - 1);
        }
        return emission_color;
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

void buffer_to_image(Image& image, const std::vector<Color3>& buffer, int width, int height, int accumulated_samples) {
    double scale = 1.0 / accumulated_samples;
    unsigned char* pixels = (unsigned char*)image.data;
    
    interval intensity(0.000, 0.999);

    for (int j = 0; j < height; j++) {
        for (int i = 0; i < width; i++) {
            int pixel_index = (j * width + i) * 4; 
            
            Color3 col = buffer[j * width + i] * scale;
            
            col.x = sqrt(col.x);
            col.y = sqrt(col.y);
            col.z = sqrt(col.z);
            
            pixels[pixel_index + 0] = (unsigned char)(256 * intensity.clamp(col.x));
            pixels[pixel_index + 1] = (unsigned char)(256 * intensity.clamp(col.y));
            pixels[pixel_index + 2] = (unsigned char)(256 * intensity.clamp(col.z));
            pixels[pixel_index + 3] = 255;
        }
    }
}

    HittableList random_scene() {
        HittableList world;

        auto checker = std::make_shared<CheckerTexture>(0.32, Color3(0.2, 0.3, 0.1), Color3(0.9, 0.9, 0.9));
        world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, std::make_shared<Lambertian>(checker)));
        auto ground_material = std::make_shared<Lambertian>(checker);
        world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, ground_material));

        auto earth_texture = std::make_shared<ImageTexture>("earthmap.png");
        auto earth_surface = std::make_shared<Lambertian>(earth_texture);
        world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, earth_surface));
        world.add(std::make_shared<Sphere>(Point3(0, 2, 0), 1.5, earth_surface));

        auto pertext = std::make_shared<NoiseTexture>(4.0);
        world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, std::make_shared<Lambertian>(pertext)));
        world.add(std::make_shared<Sphere>(Point3(0, 2, 0), 2, std::make_shared<Lambertian>(pertext)));

        for (int a = -5; a < 5; a++) {
            for (int b = -5; b < 5; b++) {
                auto choose_mat = random_double();
                Point3 center(a + 0.9 * random_double(), 0.2, b + 0.9 * random_double());

                if ((center - Point3(4, 0.2, 0)).length() > 0.9) {
                    std::shared_ptr<RTMaterial> sphere_material;

                    if (choose_mat < 0.8) {
                        auto albedo = Color3::random() * Color3::random();
                        sphere_material = std::make_shared<Lambertian>(albedo);
                        
                        auto center2 = center + Vec3(0, random_double(0, 0.5), 0);
                        world.add(std::make_shared<Sphere>(center, center2, 0.2, sphere_material));
                    } else if (choose_mat < 0.95) {
                        auto albedo = Color3::random(0.5, 1);
                        auto fuzz = random_double(0, 0.5);
                        sphere_material = std::make_shared<Metal>(albedo, fuzz);
                        world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    } else {
                        sphere_material = std::make_shared<Dielectric>(1.5);
                        world.add(std::make_shared<Sphere>(center, 0.2, sphere_material));
                    }
                }
            }
        }

        auto material1 = std::make_shared<Dielectric>(1.5);
        world.add(std::make_shared<Sphere>(Point3(0, 1, 0), 1.0, material1));

        auto material2 = std::make_shared<Lambertian>(Color3(0.4, 0.2, 0.1));
        world.add(std::make_shared<Sphere>(Point3(-4, 1, 0), 1.0, material2));

        auto material3 = std::make_shared<Metal>(Color3(0.7, 0.6, 0.5), 0.0);
        world.add(std::make_shared<Sphere>(Point3(4, 1, 0), 1.0, material3));

        return HittableList(std::make_shared<BVHNode>(world));
    }

    HittableList quads_scene() {
        HittableList world;
    
        auto left_red     = std::make_shared<Lambertian>(Color3(1.0, 0.2, 0.2));
        auto back_green   = std::make_shared<Lambertian>(Color3(0.2, 1.0, 0.2));
        auto right_blue   = std::make_shared<Lambertian>(Color3(0.2, 0.2, 1.0));
        auto upper_orange = std::make_shared<Lambertian>(Color3(1.0, 0.5, 0.0));
        auto lower_teal   = std::make_shared<Lambertian>(Color3(0.2, 0.8, 0.8));
    
        world.add(std::make_shared<Quad>(Point3(-3,-2, 5), Vec3(0, 0,-4), Vec3(0, 4, 0), left_red));
        world.add(std::make_shared<Quad>(Point3(-2,-2, 0), Vec3(4, 0, 0), Vec3(0, 4, 0), back_green));
        world.add(std::make_shared<Quad>(Point3( 3,-2, 1), Vec3(0, 0, 4), Vec3(0, 4, 0), right_blue));
        world.add(std::make_shared<Quad>(Point3(-2, 3, 1), Vec3(4, 0, 0), Vec3(0, 0, 4), upper_orange));
        world.add(std::make_shared<Quad>(Point3(-2,-3, 5), Vec3(4, 0, 0), Vec3(0, 0,-4), lower_teal));
    
        return world;
    }

    HittableList simple_light() {
        HittableList world;
    
        auto pertext = std::make_shared<NoiseTexture>(4);
        world.add(std::make_shared<Sphere>(Point3(0, -1000, 0), 1000, std::make_shared<Lambertian>(pertext)));
        world.add(std::make_shared<Sphere>(Point3(0, 2, 0), 2, std::make_shared<Lambertian>(pertext)));
    
        auto difflight = std::make_shared<DiffuseLight>(Color3(4, 4, 4));
        world.add(std::make_shared<Quad>(Point3(3, 1, -2), Vec3(2, 0, 0), Vec3(0, 2, 0), difflight));
        world.add(std::make_shared<Sphere>(Point3(0, 7, 0), 2, difflight));
        return world;
    }

    std::shared_ptr<HittableList> box(const Point3& a, const Point3& b, std::shared_ptr<RTMaterial> mat) {
        auto sides = std::make_shared<HittableList>();
    
        Point3 min(fmin(a.x, b.x), fmin(a.y, b.y), fmin(a.z, b.z));
        Point3 max(fmax(a.x, b.x), fmax(a.y, b.y), fmax(a.z, b.z));
    
        Vec3 dx(max.x - min.x, 0, 0);
        Vec3 dy(0, max.y - min.y, 0);
        Vec3 dz(0, 0, max.z - min.z);
    
        sides->add(std::make_shared<Quad>(Point3(min.x, min.y, max.z),  dx,  dy, mat)); 
        sides->add(std::make_shared<Quad>(Point3(max.x, min.y, max.z), -dz,  dy, mat)); 
        sides->add(std::make_shared<Quad>(Point3(max.x, min.y, min.z), -dx,  dy, mat)); 
        sides->add(std::make_shared<Quad>(Point3(min.x, min.y, min.z),  dz,  dy, mat)); 
        sides->add(std::make_shared<Quad>(Point3(min.x, max.y, max.z),  dx, -dz, mat)); 
        sides->add(std::make_shared<Quad>(Point3(min.x, min.y, min.z),  dx,  dz, mat));
    
        return sides;
    }

    HittableList cornell_box() {
        HittableList world;
    
        auto red   = std::make_shared<Lambertian>(Color3(0.65, 0.05, 0.05));
        auto white = std::make_shared<Lambertian>(Color3(0.73, 0.73, 0.73));
        auto green = std::make_shared<Lambertian>(Color3(0.12, 0.45, 0.15));
        auto light = std::make_shared<DiffuseLight>(Color3(15, 15, 15)); 
    
        
        world.add(std::make_shared<Quad>(Point3(555, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), green));
        world.add(std::make_shared<Quad>(Point3(0, 0, 0), Vec3(0, 555, 0), Vec3(0, 0, 555), red));
        world.add(std::make_shared<Quad>(Point3(343, 554, 332), Vec3(-130, 0, 0), Vec3(0, 0, -105), light));
        world.add(std::make_shared<Quad>(Point3(0, 0, 0), Vec3(555, 0, 0), Vec3(0, 0, 555), white));
        world.add(std::make_shared<Quad>(Point3(555, 555, 555), Vec3(-555, 0, 0), Vec3(0, 0, -555), white));
        world.add(std::make_shared<Quad>(Point3(0, 0, 555), Vec3(555, 0, 0), Vec3(0, 555, 0), white));


        world.add(box(Point3(130, 0, 65), Point3(295, 165, 230), white));
        world.add(box(Point3(265, 0, 295), Point3(430, 330, 460), white));

        std::shared_ptr<Hittable> box1 = box(Point3(0, 0, 0), Point3(165, 330, 165), white);
        box1 = std::make_shared<RotateY>(box1, 15);
        box1 = std::make_shared<Translate>(box1, Vec3(265, 0, 295));

        std::shared_ptr<Hittable> box2 = box(Point3(0, 0, 0), Point3(165, 165, 165), white);
        box2 = std::make_shared<RotateY>(box2, -18);
        box2 = std::make_shared<Translate>(box2, Vec3(130, 0, 65));

        world.add(std::make_shared<ConstantMedium>(box1, 0.01, Color3(0, 0, 0))); 
        world.add(std::make_shared<ConstantMedium>(box2, 0.01, Color3(1, 1, 1)));

    return world;
    }

    HittableList final_scene() {
        HittableList boxes1;
        auto ground = std::make_shared<Lambertian>(Color3(0.48, 0.83, 0.53));
        int boxes_per_side = 5;
        for (int i = 0; i < boxes_per_side; i++) {
            for (int j = 0; j < boxes_per_side; j++) {
                auto w = 100.0;
                auto x0 = -1000.0 + i * w;
                auto z0 = -1000.0 + j * w;
                auto y0 = 0.0;
                auto x1 = x0 + w;
                auto y1 = random_double(1, 101); 
                auto z1 = z0 + w;
    
                boxes1.add(box(Point3(x0, y0, z0), Point3(x1, y1, z1), ground));
            }
        }
        HittableList world;
    
        world.add(std::make_shared<BVHNode>(boxes1));
    
        auto light = std::make_shared<DiffuseLight>(Color3(7, 7, 7));
        world.add(std::make_shared<Quad>(Point3(123, 554, 147), Vec3(300, 0, 0), Vec3(0, 0, 265), light));
    
        auto center1 = Point3(400, 400, 200);
        auto center2 = center1 + Vec3(30, 0, 0); 
        auto sphere_material = std::make_shared<Lambertian>(Color3(0.7, 0.3, 0.1));
        world.add(std::make_shared<Sphere>(center1, center2, 50, sphere_material));
    
        world.add(std::make_shared<Sphere>(Point3(260, 150, 45), 50, std::make_shared<Dielectric>(1.5)));
    
        world.add(std::make_shared<Sphere>(Point3(0, 150, 145), 50, std::make_shared<Metal>(Color3(0.8, 0.8, 0.9), 1.0)));
    
        auto boundary = std::make_shared<Sphere>(Point3(360, 150, 145), 70, std::make_shared<Dielectric>(1.5));
        world.add(boundary);
        world.add(std::make_shared<ConstantMedium>(boundary, 0.2, Color3(0.2, 0.4, 0.9)));
    
        boundary = std::make_shared<Sphere>(Point3(0, 0, 0), 5000, std::make_shared<Dielectric>(1.5));
        world.add(std::make_shared<ConstantMedium>(boundary, .0001, Color3(1, 1, 1)));
    
        auto earth_texture = std::make_shared<ImageTexture>("earthmap.png");
        auto earth_mat = std::make_shared<Lambertian>(earth_texture);
        world.add(std::make_shared<Sphere>(Point3(400, 200, 400), 100, earth_mat));
    
        auto pertext = std::make_shared<NoiseTexture>(0.1);
        world.add(std::make_shared<Sphere>(Point3(220, 280, 300), 80, std::make_shared<Lambertian>(pertext)));
    
        HittableList boxes2;
        auto white = std::make_shared<Lambertian>(Color3(0.73, 0.73, 0.73));
        int ns = 10;
        for (int j = 0; j < ns; j++) {
            boxes2.add(std::make_shared<Sphere>(Point3::random(0, 165), 10, white));
        }
    
        world.add(std::make_shared<Translate>(
            std::make_shared<RotateY>(
                std::make_shared<BVHNode>(boxes2), 15),
                Vec3(-100, 270, 395)
            )
        );
    
        return world;
    }

int main() {
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    srand(static_cast<unsigned int>(time(NULL)));

    const int screen_width = 200;
    const int screen_height = 200;
    const double aspect_ratio = 1.0;

    InitWindow(screen_width, screen_height, "Ray Tracing: The Next Week (Raylib)");
    SetTargetFPS(60);

    int samples_per_pixel = 50; 
    int max_depth = 8;         
    bool is_rendering = true;
    int accumulated_samples = 0;

    Point3 lookfrom(478, 278, -600);
    Point3 lookat(278, 278, 0);
    Vec3 vup(0, 1, 0);

    double vfov = 40.0;      
    double dist_to_focus = 10.0; 
    double aperture = 0.0;

    RTCamera camera(lookfrom, lookat, vup, vfov, aspect_ratio, aperture, dist_to_focus, 0.0, 1.0);

    std::vector<Color3> accumulation_buffer(screen_width * screen_height, Color3(0, 0, 0));
    Image render_image = GenImageColor(screen_width, screen_height, BLACK);
    Texture2D render_texture = LoadTextureFromImage(render_image);

    HittableList world = final_scene();

    float move_speed = 10.0f;
    float mouse_sensitivity = 0.003f;

    samples_per_pixel = 1;

    bool camera_moved = false;
    Vector2 last_mouse_pos = GetMousePosition();

    DisableCursor();

    while (!WindowShouldClose()) {
        camera_moved = false;

        if (IsKeyDown(KEY_W)) { 
            camera.move_forward(move_speed); 
            camera_moved = true; 
        }
        if (IsKeyDown(KEY_S)) { 
            camera.move_forward(-move_speed); 
            camera_moved = true; 
        }
        if (IsKeyDown(KEY_A)) { 
            camera.move_right(-move_speed); 
            camera_moved = true; 
        }
        if (IsKeyDown(KEY_D)) { 
            camera.move_right(move_speed); 
            camera_moved = true; 
        }
        if (IsKeyDown(KEY_SPACE)) { 
            camera.move_up(move_speed); 
            camera_moved = true; 
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) { 
            camera.move_up(-move_speed); 
            camera_moved = true; 
        }

        Vector2 mouse_pos = GetMousePosition();
        Vector2 mouse_delta = {mouse_pos.x - last_mouse_pos.x, mouse_pos.y - last_mouse_pos.y};
        last_mouse_pos = mouse_pos;

        if (mouse_delta.x != 0 || mouse_delta.y != 0) {
            camera.rotate(-mouse_delta.x * mouse_sensitivity, -mouse_delta.y * mouse_sensitivity);
            camera_moved = true;
        }

        if (IsKeyPressed(KEY_P)) is_rendering = !is_rendering;
        if (IsKeyPressed(KEY_R)) camera_moved = true;
        
        if (IsKeyPressed(KEY_EQUAL) || IsKeyPressed(KEY_KP_ADD)) 
            samples_per_pixel = (samples_per_pixel + 1 < 10) ? samples_per_pixel + 1 : 10;
        if (IsKeyPressed(KEY_MINUS) || IsKeyPressed(KEY_KP_SUBTRACT)) 
            samples_per_pixel = (samples_per_pixel - 1 > 1) ? samples_per_pixel - 1 : 1;

        if (camera_moved) {
            accumulated_samples = 0;
            std::fill(accumulation_buffer.begin(), accumulation_buffer.end(), Color3(0,0,0));
             for (int j = 0; j < screen_height; j++) {
                for (int i = 0; i < screen_width; i++) {
                    double u = (i + 0.5) / (screen_width - 1);
                    double v = (j + 0.5) / (screen_height - 1);
                    RTRay r = camera.get_ray(u, 1.0 - v);
                    accumulation_buffer[j * screen_width + i] = ray_color(r, world, 2);
                }
            }
            accumulated_samples = 1;
            buffer_to_image(render_image, accumulation_buffer, screen_width, screen_height, 1);
            UpdateTexture(render_texture, render_image.data);
        }
        else if (is_rendering) {
            for (int j = 0; j < screen_height; j++) {
                for (int i = 0; i < screen_width; i++) {
                    Color3 pixel_color(0, 0, 0);
                    for (int s = 0; s < samples_per_pixel; s++) {
                        double u = (i + random_double()) / (screen_width - 1);
                        double v = (j + random_double()) / (screen_height - 1);
                        RTRay r = camera.get_ray(u, 1.0 - v);
                        pixel_color += ray_color(r, world, max_depth);
                    }
                    accumulation_buffer[j * screen_width + i] += pixel_color;
                }
            }
            accumulated_samples += samples_per_pixel;
            buffer_to_image(render_image, accumulation_buffer, screen_width, screen_height, accumulated_samples);
            UpdateTexture(render_texture, render_image.data);
        }

        BeginDrawing();
        ClearBackground(BLACK);
        DrawTexture(render_texture, 0, 0, WHITE);
        
        DrawText(TextFormat("FPS: %d", GetFPS()), 10, 10, 20, GREEN);
        DrawText(TextFormat("Samples: %d", accumulated_samples), 10, 35, 20, GREEN);
        DrawText(is_rendering ? "Rendering..." : "PAUSED", 10, 60, 20, is_rendering ? GREEN : RED);
        
        EndDrawing();
    }

    UnloadTexture(render_texture);
    UnloadImage(render_image);
    CloseWindow();
    return 0;
}