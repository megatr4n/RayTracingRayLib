#include "raylib.h"

#include "rtweekend.h"
#include "hittable.h" 
#include "camera.h"   
#include "material.h"
#include "quad.h"
#include "pdf.h" 

#include <cmath>
#include <vector>
#include <memory>

inline double clamp(double x, double min, double max) {
    if (x < min) return min;
    if (x > max) return max;
    return x;
}

inline std::shared_ptr<HittableList> box(const Point3& a, const Point3& b, std::shared_ptr<RTMaterial> mat) {
    auto sides = std::make_shared<HittableList>();
    auto min = Point3(std::fmin(a.x, b.x), std::fmin(a.y, b.y), std::fmin(a.z, b.z));
    auto max = Point3(std::fmax(a.x, b.x), std::fmax(a.y, b.y), std::fmax(a.z, b.z));
    Vec3 dx = Vec3(max.x - min.x, 0, 0);
    Vec3 dy = Vec3(0, max.y - min.y, 0);
    Vec3 dz = Vec3(0, 0, max.z - min.z);
    sides->add(std::make_shared<Quad>(Point3(min.x, min.y, max.z),  dx,  dy, mat)); 
    sides->add(std::make_shared<Quad>(Point3(max.x, min.y, max.z), -dz,  dy, mat)); 
    sides->add(std::make_shared<Quad>(Point3(max.x, min.y, min.z), -dx,  dy, mat)); 
    sides->add(std::make_shared<Quad>(Point3(min.x, min.y, min.z),  dz,  dy, mat)); 
    sides->add(std::make_shared<Quad>(Point3(min.x, max.y, max.z),  dx, -dz, mat)); 
    sides->add(std::make_shared<Quad>(Point3(min.x, min.y, min.z),  dx,  dz, mat)); 
    return sides;
}

Color3 ray_color(const RTRay& r, int depth, const Hittable& world, const Hittable& lights) {
    if (depth <= 0) return Color3(0,0,0);

    HitRecord rec;
    if (!world.hit(r, interval(0.001, infinity), rec))
        return Color3(0,0,0);

    ScatterRecord srec;
    
    Color3 color_from_emission = rec.mat->emitted(r, rec, rec.u, rec.v, rec.p);

    if (!rec.mat->scatter(r, rec, srec))
        return color_from_emission;

    if (srec.skip_pdf) {
        return srec.attenuation * ray_color(srec.skip_pdf_ray, depth-1, world, lights);
    }

    auto light_ptr = std::make_shared<HittablePdf>(lights, rec.p);
    MixturePdf mixed_pdf(light_ptr, srec.pdf_ptr);

    RTRay scattered = RTRay(rec.p, mixed_pdf.generate(), r.tm);
    double pdf_val = mixed_pdf.value(scattered.direction);

    double scattering_pdf = rec.mat->scattering_pdf(r, rec, scattered);

    if (pdf_val == 0) return color_from_emission;

    return color_from_emission + 
           (srec.attenuation * scattering_pdf * ray_color(scattered, depth-1, world, lights)) / pdf_val;
}

int main() {
    const int screenWidth = 600;
    const int screenHeight = 600;
    const int max_depth = 50; 

    InitWindow(screenWidth, screenHeight, "Ray Tracer Book 3: Final");
    SetTargetFPS(60);
    DisableCursor();

    HittableList world;

    auto red   = std::make_shared<Lambertian>(Color3(.65, .05, .05));
    auto white = std::make_shared<Lambertian>(Color3(.73, .73, .73));
    auto green = std::make_shared<Lambertian>(Color3(.12, .45, .15));
    auto light = std::make_shared<DiffuseLight>(Color3(15, 15, 15));
    
    auto aluminum = std::make_shared<Metal>(Color3(0.8, 0.85, 0.88), 0.5);
    auto glass    = std::make_shared<Dielectric>(1.5);

    world.add(std::make_shared<Quad>(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.add(std::make_shared<Quad>(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.add(std::make_shared<Quad>(Point3(343, 554, 332), Vec3(-130,0,0), Vec3(0,0,-105), light)); // Лампа
    world.add(std::make_shared<Quad>(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white)); // Пол
    world.add(std::make_shared<Quad>(Point3(555,555,555), Vec3(-555,0,0), Vec3(0,0,-555), white)); // Потолок
    world.add(std::make_shared<Quad>(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white)); // Задняя стена

    std::shared_ptr<Hittable> box1 = box(Point3(130, 0, 65), Point3(295, 165, 230), white); // Обычный белый куб
    
    world.add(std::make_shared<Sphere>(Point3(190, 90, 190), 90, glass));

    std::shared_ptr<Hittable> box2 = box(Point3(265, 0, 295), Point3(430, 330, 460), aluminum);
    world.add(box2);

    HittableList lights;
    lights.add(std::make_shared<Quad>(Point3(343, 554, 332), Vec3(-130,0,0), Vec3(0,0,-105), light));

    RTCamera cam(
        Point3(278, 278, -800),
        Point3(278, 278, 0),
        Vec3(0, 1, 0),
        40.0,
        double(screenWidth)/screenHeight,
        0.0,
        10.0
    );

    Image image = GenImageColor(screenWidth, screenHeight, BLACK);
    Texture2D texture = LoadTextureFromImage(image);
    
    std::vector<Color3> accumBuffer(screenWidth * screenHeight, Color3(0,0,0));
    int framesAccumulated = 0;

    while (!WindowShouldClose()) {
        double speed = 5.0; 
        double sensitivity = 0.002;
        bool cameraMoved = false;

        if (IsKeyDown(KEY_W)) { cam.move_forward(speed); cameraMoved = true; }
        if (IsKeyDown(KEY_S)) { cam.move_forward(-speed); cameraMoved = true; }
        if (IsKeyDown(KEY_A)) { cam.move_right(-speed); cameraMoved = true; }
        if (IsKeyDown(KEY_D)) { cam.move_right(speed); cameraMoved = true; }
        if (IsKeyDown(KEY_SPACE)) { cam.move_up(speed); cameraMoved = true; }
        if (IsKeyDown(KEY_LEFT_SHIFT)) { cam.move_up(-speed); cameraMoved = true; }

        Vector2 mouseDelta = GetMouseDelta();
        if (mouseDelta.x != 0 || mouseDelta.y != 0) {
            cam.rotate(mouseDelta.x * sensitivity, mouseDelta.y * sensitivity); 
            cameraMoved = true;
        }

        if (cameraMoved) {
            framesAccumulated = 0;
            std::fill(accumBuffer.begin(), accumBuffer.end(), Color3(0,0,0));
        }

        framesAccumulated++;

        #pragma omp parallel for 
        for (int j = 0; j < screenHeight; ++j) {
            for (int i = 0; i < screenWidth; ++i) {
                double u = (double(i) + random_double()) / (screenWidth - 1);
                double v = (double(screenHeight - 1 - j) + random_double()) / (screenHeight - 1);

                RTRay r = cam.get_ray(u, v);
                Color3 pixel_color = ray_color(r, max_depth, world, lights);

                int pixelIndex = j * screenWidth + i;
                accumBuffer[pixelIndex] += pixel_color;
                Color3 accumulatedColor = accumBuffer[pixelIndex] / double(framesAccumulated);

                if (accumulatedColor.x != accumulatedColor.x) accumulatedColor = Color3(0,0,0);
                if (accumulatedColor.y != accumulatedColor.y) accumulatedColor = Color3(0,0,0);
                if (accumulatedColor.z != accumulatedColor.z) accumulatedColor = Color3(0,0,0);

                auto r_val = sqrt(accumulatedColor.x);
                auto g_val = sqrt(accumulatedColor.y);
                auto b_val = sqrt(accumulatedColor.z);

                Color finalColor;
                finalColor.r = (unsigned char)(256 * clamp(r_val, 0.0, 0.999));
                finalColor.g = (unsigned char)(256 * clamp(g_val, 0.0, 0.999));
                finalColor.b = (unsigned char)(256 * clamp(b_val, 0.0, 0.999));
                finalColor.a = 255;

                ((Color*)image.data)[pixelIndex] = finalColor;
            }
        }
        
        UpdateTexture(texture, image.data);

        BeginDrawing();
            ClearBackground(BLACK);
            DrawTexture(texture, 0, 0, WHITE);
            DrawFPS(10, 10);
            DrawText(TextFormat("Samples: %d", framesAccumulated), 10, 30, 20, GREEN);
        EndDrawing();
    }

    UnloadImage(image);
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}