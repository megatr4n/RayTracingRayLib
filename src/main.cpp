#include <raylib.h>
#include <rlImGui.h>
#include <imgui.h>
#include <algorithm>

#include "renderer.h"
#include "hittable.h"
#include "random_utils.h"
#include "ui.h"

using namespace rt;

static Scene buildScene() {
    Scene scene;
    srand(42);

    { Sphere s; s.center={0,-1000,0}; s.radius=1000;
      s.mat.type=MaterialType::Lambertian; s.mat.albedo={0.5f,0.5f,0.5f};
      scene.spheres.push_back(s); }
    { Sphere s; s.center={0,1,0}; s.radius=1;
      s.mat.type=MaterialType::Dielectric; s.mat.ior=1.5f;
      scene.spheres.push_back(s); }
    { Sphere s; s.center={-4,1,0}; s.radius=1;
      s.mat.type=MaterialType::Lambertian; s.mat.albedo={0.4f,0.2f,0.1f};
      scene.spheres.push_back(s); }
    { Sphere s; s.center={4,1,0}; s.radius=1;
      s.mat.type=MaterialType::Metal; s.mat.albedo={0.7f,0.6f,0.5f}; s.mat.fuzz=0.0f;
      scene.spheres.push_back(s); }

    for (int a = -5; a < 5; a++) {
        for (int b = -5; b < 5; b++) {
            Vec3 center = {a + 0.9f*randomFloat(), 0.2f, b + 0.9f*randomFloat()};
            if ((center - Vec3{4,0.2f,0}).length() <= 0.9f) continue;
            Sphere s; s.center=center; s.radius=0.2f;
            float r = randomFloat();
            if (r < 0.6f) {
                s.mat.type=MaterialType::Lambertian;
                s.mat.albedo=randomVec3()*randomVec3();
            } else if (r < 0.85f) {
                s.mat.type=MaterialType::Metal;
                s.mat.albedo=randomVec3(0.5f,1.0f);
                s.mat.fuzz=randomFloat(0.0f,0.3f);
            } else {
                s.mat.type=MaterialType::Dielectric; s.mat.ior=1.5f;
            }
            scene.spheres.push_back(s);
        }
    }
    return scene;
}
Camera3D ConvertCamera(const RtCameraParams& params) {
  Camera3D cam = { 0 };
  cam.position   = { 
    params.lookFrom.x, params.lookFrom.y, params.lookFrom.z 
  };
  cam.target     = { 
    params.lookAt.x, params.lookAt.y, params.lookAt.z 
  };
  cam.up         = { 
    params.vUp.x, params.vUp.y, params.vUp.z 
  };
  cam.fovy       = params.vfov;
  cam.projection = CAMERA_PERSPECTIVE;
  return cam;
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
        renderer.update(GetFrameTime());

        renderer.renderSample();
        if (renderer.isPreview) {
          Camera3D rlCam = ConvertCamera(renderer.camParams);
          BeginMode3D(rlCam);
              
              DrawGrid(20, 1.0f);

              for (const auto& s : scene.spheres) {
                  Vector3 center = { s.center.x, s.center.y, s.center.z };
                  Vector3 size = { s.radius * 2.0f, s.radius * 2.0f, s.radius * 2.0f };
                  DrawCubeWiresV(center, size, WHITE);
              }

          EndMode3D();
          
          DrawText("PREVIEW MODE (WASD to Fly)", 10, 10, 20, GREEN);
      } else {
           DrawText(TextFormat("Rendering... Sample: %d", renderer.samplesDone), 10, 10, 20, ORANGE);
      }

        BeginDrawing();
        ClearBackground(BLACK);

        int   panelW = GetScreenWidth() - 340;
        int   panelH = GetScreenHeight();
        float scale  = std::min((float)panelW / renderer.settings.width,
                                (float)panelH / renderer.settings.height);
        int dw = (int)(renderer.settings.width  * scale);
        int dh = (int)(renderer.settings.height * scale);
        Rectangle srcRect = { 0, 0, (float)renderer.outputTex.width, -(float)renderer.outputTex.height };
        Rectangle destRect = { 0, 0, (float)GetScreenWidth(), (float)GetScreenHeight() };
        DrawTexturePro(renderer.outputTex, srcRect, destRect, {0,0}, 0.0f, WHITE);

        rlImGuiBegin();
        ImGui::SetNextWindowPos ({(float)(GetScreenWidth()-340), 0}, ImGuiCond_Always);
        ImGui::SetNextWindowSize({340, (float)GetScreenHeight()},    ImGuiCond_Always);
        if (drawUI(renderer, scene)) renderer.reset();
        rlImGuiEnd();

        EndDrawing();
    }

    rlImGuiShutdown();
    UnloadTexture(renderer.outputTex);
    CloseWindow();
    return 0;
}