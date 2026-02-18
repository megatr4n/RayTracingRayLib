#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>
#include <algorithm>

#include "renderer.h"
#include "hittable.h"
#include "random_utils.h"
#include "ui.h"

static Scene buildScene() {
    Scene scene;
    srand(42);

    { Sphere s; s.center = {0, -1000, 0}; s.radius = 1000;
      s.mat.type = MaterialType::Lambertian; s.mat.albedo = {0.5f, 0.5f, 0.5f};
      scene.spheres.push_back(s); }

    { Sphere s; s.center = {0,1,0}; s.radius = 1;
      s.mat.type = MaterialType::Dielectric; s.mat.ior = 1.5f;
      scene.spheres.push_back(s); }

    { Sphere s; s.center = {-4,1,0}; s.radius = 1;
      s.mat.type = MaterialType::Lambertian; s.mat.albedo = {0.4f, 0.2f, 0.1f};
      scene.spheres.push_back(s); }

    { Sphere s; s.center = {4,1,0}; s.radius = 1;
      s.mat.type = MaterialType::Metal;
      s.mat.albedo = {0.7f, 0.6f, 0.5f}; s.mat.fuzz = 0.0f;
      scene.spheres.push_back(s); }

    for (int a = -5; a < 5; a++) {
        for (int b = -5; b < 5; b++) {
            Vec3 center = { a + 0.9f * randomFloat(), 0.2f, b + 0.9f * randomFloat() };
            if ((center - Vec3{4, 0.2f, 0}).length() <= 0.9f) continue;

            Sphere s; s.center = center; s.radius = 0.2f;
            float r = randomFloat();

            if (r < 0.6f) {
                s.mat.type   = MaterialType::Lambertian;
                s.mat.albedo = randomVec3() * randomVec3();
            } else if (r < 0.85f) {
                s.mat.type   = MaterialType::Metal;
                s.mat.albedo = randomVec3(0.5f, 1.0f);
                s.mat.fuzz   = randomFloat(0.0f, 0.3f);
            } else {
                s.mat.type = MaterialType::Dielectric;
                s.mat.ior  = 1.5f;
            }
            scene.spheres.push_back(s);
        }
    }
    return scene;
}

int main() {
    SetConfigFlags(FLAG_WINDOW_RESIZABLE | FLAG_VSYNC_HINT);
    InitWindow(1280, 720, "Real-Time Raytracer | raylib + ImGui");
    SetTargetFPS(60);
    rlImGuiSetup(true);

    Scene    scene = buildScene();
    Renderer renderer;

    renderer.settings.width         = 800;
    renderer.settings.height        = 450;
    renderer.settings.maxBounces    = 8;
    renderer.settings.samplesTarget = 256;
    renderer.camParams.vfov         = 20.0f;
    renderer.camParams.aperture     = 0.1f;
    renderer.camParams.focusDist    = 10.0f;
    renderer.camParams.lookFrom     = {13, 2, 3};
    renderer.camParams.lookAt       = {0, 0, 0};
    renderer.init(&scene);

    while (!WindowShouldClose()) {
        renderer.renderSample();

        BeginDrawing();
        ClearBackground(BLACK);

        int   panelW = GetScreenWidth() - 340;
        int   panelH = GetScreenHeight();
        float scale  = std::min((float)panelW / renderer.settings.width,
                                (float)panelH / renderer.settings.height);
        int dw = (int)(renderer.settings.width  * scale);
        int dh = (int)(renderer.settings.height * scale);

        Rectangle src = { 0, 0, (float)renderer.settings.width,
                                 (float)renderer.settings.height };
        Rectangle dst = { (panelW - dw) / 2.0f, (panelH - dh) / 2.0f,
                          (float)dw, (float)dh };

        DrawTexturePro(renderer.outputTex, src, dst, {0, 0}, 0, WHITE);

        rlImGuiBegin();
        ImGui::SetNextWindowPos ({(float)(GetScreenWidth() - 340), 0},
                                  ImGuiCond_Always);
        ImGui::SetNextWindowSize({340, (float)GetScreenHeight()},
                                  ImGuiCond_Always);
        if (drawUI(renderer, scene)) renderer.reset();
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    UnloadTexture(renderer.outputTex);
    CloseWindow();
    return 0;
}