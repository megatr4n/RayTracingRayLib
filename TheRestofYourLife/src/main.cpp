#include "raylib.h"

#include "rtweekend.h"
#include "hittable.h" 
#include "camera.h"   
#include "material.h"
#include "quad.h"     

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

Color3 ray_color(const RTRay& r, int depth, const Hittable& world) {
    if (depth <= 0) return Color3(0,0,0);

    HitRecord rec;
    if (!world.hit(r, interval(0.01, infinity), rec))
        return Color3(0,0,0);

    RTRay scattered;
    Color3 attenuation;
    Color3 color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);

    if (!rec.mat->scatter(r, rec, attenuation, scattered))
        return color_from_emission;

    return color_from_emission + attenuation * ray_color(scattered, depth-1, world);
}

int main() {
    const int screenWidth = 600;
    const int screenHeight = 600;
    const int max_depth = 5;         

    InitWindow(screenWidth, screenHeight, "Book 3: Accumulation Buffer");
    SetTargetFPS(60);
    DisableCursor();

    HittableList world;

    auto red   = std::make_shared<Lambertian>(Color3(.65, .05, .05));
    auto white = std::make_shared<Lambertian>(Color3(.73, .73, .73));
    auto green = std::make_shared<Lambertian>(Color3(.12, .45, .15));
    auto light = std::make_shared<DiffuseLight>(Color3(15, 15, 15));

    world.add(std::make_shared<Quad>(Point3(555,0,0), Vec3(0,555,0), Vec3(0,0,555), green));
    world.add(std::make_shared<Quad>(Point3(0,0,0), Vec3(0,555,0), Vec3(0,0,555), red));
    world.add(std::make_shared<Quad>(Point3(343, 554, 332), Vec3(-130,0,0), Vec3(0,0,-105), light));
    world.add(std::make_shared<Quad>(Point3(0,0,0), Vec3(555,0,0), Vec3(0,0,555), white));
    world.add(std::make_shared<Quad>(Point3(555,555,555), Vec3(-555,0,0), Vec3(0,0,-555), white));
    world.add(std::make_shared<Quad>(Point3(0,0,555), Vec3(555,0,0), Vec3(0,555,0), white));

    std::shared_ptr<Hittable> box1 = box(Point3(130, 0, 65), Point3(295, 165, 230), white);
    std::shared_ptr<Hittable> box2 = box(Point3(265, 0, 295), Point3(430, 330, 460), white);
    world.add(box1);
    world.add(box2);

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

        if (IsKeyDown(KEY_W)) { 
            cam.move_forward(speed); 
            cameraMoved = true; 
        }
        if (IsKeyDown(KEY_S)) { 
            cam.move_forward(-speed); 
            cameraMoved = true; 
        }
        if (IsKeyDown(KEY_A)) { 
            cam.move_right(-speed); 
            cameraMoved = true; 
        }
        if (IsKeyDown(KEY_D)) { 
            cam.move_right(speed); 
            cameraMoved = true; 
        }
        if (IsKeyDown(KEY_SPACE)) { 
            cam.move_up(speed); 
            cameraMoved = true; 
        }
        if (IsKeyDown(KEY_LEFT_SHIFT)) { 
            cam.move_up(-speed); 
            cameraMoved = true; 
        }

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
                Color3 pixel_color = ray_color(r, max_depth, world);

                int pixelIndex = j * screenWidth + i;
                accumBuffer[pixelIndex] += pixel_color;

                Color3 accumulatedColor = accumBuffer[pixelIndex] / double(framesAccumulated);

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
            
            DrawText(TextFormat("Quality: %d samples", framesAccumulated), 10, 30, 20, GREEN);
            
            if (framesAccumulated < 10) 
                DrawText("Moving... (Low Res)", 10, 50, 20, RED);
            else 
                DrawText("Refining Image...", 10, 50, 20, YELLOW);
                
        EndDrawing();
    }

    UnloadImage(image);
    UnloadTexture(texture);
    CloseWindow();

    return 0;
}