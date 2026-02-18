#include "ui.h"
#include "renderer.h" 
#include "random_utils.h"
#include <imgui.h>
#include <string>
#include <vector>

namespace rt {

// === ФУНКЦІЯ: Створює класичну рандомну сцену ===
void createRandomScene(Scene& scene, RtCameraParams& cam) {
    scene.spheres.clear();
    
    // Земля
    scene.spheres.push_back({ {0,-1000,0}, 1000, {MaterialType::Lambertian, {0.5f,0.5f,0.5f}} });

    // Великі кулі
    scene.spheres.push_back({ {0,1,0}, 1, {MaterialType::Dielectric, {1.0f,1.0f,1.0f}, 0.0f, 1.5f} });
    scene.spheres.push_back({ {-4,1,0}, 1, {MaterialType::Lambertian, {0.4f,0.2f,0.1f}} });
    scene.spheres.push_back({ {4,1,0}, 1, {MaterialType::Metal, {0.7f,0.6f,0.5f}, 0.0f} });

    // Маленькі кулі
    for (int a = -11; a < 11; a++) {
        for (int b = -11; b < 11; b++) {
            float choose_mat = randomFloat();
            Vec3 center = {a + 0.9f*randomFloat(), 0.2f, b + 0.9f*randomFloat()};
            if ((center - Vec3{4,0.2f,0}).length() > 0.9f) {
                Sphere s; s.center = center; s.radius = 0.2f;
                if (choose_mat < 0.8f) {
                    s.mat.type = MaterialType::Lambertian;
                    s.mat.albedo = randomVec3() * randomVec3();
                } else if (choose_mat < 0.95f) {
                    s.mat.type = MaterialType::Metal;
                    s.mat.albedo = randomVec3(0.5f, 1.0f);
                    s.mat.fuzz = randomFloat(0.0f, 0.5f);
                } else {
                    s.mat.type = MaterialType::Dielectric;
                    s.mat.ior = 1.5f;
                }
                scene.spheres.push_back(s);
            }
        }
    }

    // Повертаємо камеру на вулицю
    cam.lookFrom = {13, 2, 3};
    cam.lookAt   = {0, 0, 0};
    cam.vfov     = 20.0f;
    cam.aperture = 0.1f;
    cam.focusDist = 10.0f;
}

// === ФУНКЦІЯ: Створює Cornell Box зі сфер ===
void createCornellBox(Scene& scene, RtCameraParams& cam) {
    scene.spheres.clear();

    float R = 10000.0f; // Радіус стін

    // Стіни (велетенські сфери)
    // Ліва (Червона)
    scene.spheres.push_back({ {-R - 2.0f, 0, 0}, R, {MaterialType::Lambertian, {0.6f, 0.1f, 0.1f}} });
    
    // Права (Зелена)
    scene.spheres.push_back({ { R + 2.0f, 0, 0}, R, {MaterialType::Lambertian, {0.1f, 0.6f, 0.1f}} });
    
    // Задня (Біла)
    scene.spheres.push_back({ {0, 0, -R - 2.0f}, R, {MaterialType::Lambertian, {0.8f, 0.8f, 0.8f}} });
    
    // Підлога (Біла)
    scene.spheres.push_back({ {0, -R - 2.0f, 0}, R, {MaterialType::Lambertian, {0.8f, 0.8f, 0.8f}} });
    
    // Стеля (Біла)
    scene.spheres.push_back({ {0,  R + 2.0f, 0}, R, {MaterialType::Lambertian, {0.8f, 0.8f, 0.8f}} });

    // Об'єкти всередині
    // Дзеркальна куля
    scene.spheres.push_back({ {-1.0f, -1.0f, -0.5f}, 1.0f, {MaterialType::Metal, {0.9f, 0.9f, 0.9f}, 0.0f} });
    
    // Скляна куля
    scene.spheres.push_back({ { 1.0f, -1.0f, 0.5f}, 1.0f, {MaterialType::Dielectric, {1.0f, 1.0f, 1.0f}, 0.0f, 1.5f} });

    // Ставимо камеру всередину коробки
    cam.lookFrom = {0, 0, 6}; // Дивимось з "відкритої" сторони
    cam.lookAt   = {0, 0, 0};
    cam.vfov     = 40.0f;     // Трохи ширший кут огляду
    cam.aperture = 0.0f;      // Щоб все було у фокусі
    cam.focusDist = 6.0f;
}

bool drawUI(Renderer& renderer, Scene& scene) {
    bool changed = false;

    ImGui::SetNextWindowPos ({10, 10},   ImGuiCond_Once);
    ImGui::SetNextWindowSize({340, 650}, ImGuiCond_Once); // Трохи збільшив висоту
    
    ImGui::Begin("Raytracer Controls");

    // === КНОПКИ СТАРТ / СТОП ===
    if (renderer.isRendering) {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
        if (ImGui::Button("STOP RENDER", ImVec2(-1, 30))) {
            renderer.stopRender();
        }
        ImGui::PopStyleColor();
    } else {
        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.6f, 0.6f));
        if (ImGui::Button("START RENDER", ImVec2(-1, 30))) {
            renderer.startRender();
        }
        ImGui::PopStyleColor();
    }
    ImGui::Separator();
    
    // === ВИБІР СЦЕНИ (НОВЕ!) ===
    // Показуємо кнопки тільки якщо не йде рендер
    if (!renderer.isRendering) {
        ImGui::Text("Switch Scene:");
        if (ImGui::Button("Random Spheres", ImVec2(150, 0))) {
            createRandomScene(scene, renderer.camParams);
            renderer.camera.init(renderer.camParams); // Оновлюємо камеру
            renderer.reset(); // Скидаємо картинку
        }
        ImGui::SameLine();
        if (ImGui::Button("Cornell Box", ImVec2(150, 0))) {
            createCornellBox(scene, renderer.camParams);
            // Тут треба скинути yaw/pitch, щоб камера дивилась прямо
            renderer.camParams.yaw = -90.0f; // Дивимось вздовж Z
            renderer.camParams.pitch = 0.0f;
            
            renderer.camera.init(renderer.camParams);
            renderer.reset();
        }
        ImGui::Separator();
    }

    // === ПРОГРЕС ===
    float progress = (float)renderer.samplesDone / renderer.settings.samplesTarget;
    ImGui::TextColored({0.4f,1.0f,0.4f,1.0f}, "Samples: %d / %d",
                       renderer.samplesDone, renderer.settings.samplesTarget);
    ImGui::ProgressBar(progress, {-1, 0});
    ImGui::Separator();

    if (renderer.isRendering) ImGui::BeginDisabled();

    // === НАЛАШТУВАННЯ ===
    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= ImGui::SliderInt("Max Bounces",    &renderer.settings.maxBounces,    1, 32);
        changed |= ImGui::SliderInt("Target Samples", &renderer.settings.samplesTarget, 1, 2048);
    }

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        RtCameraParams& cp = renderer.camParams;
        changed |= ImGui::SliderFloat ("FOV",       &cp.vfov,       5.0f, 120.0f);
        changed |= ImGui::SliderFloat ("Aperture",  &cp.aperture,   0.0f, 2.0f);
        changed |= ImGui::SliderFloat ("Focus Dist",&cp.focusDist,  0.5f, 50.0f);
        changed |= ImGui::DragFloat3("Look From", &cp.lookFrom.x, 0.1f);
        changed |= ImGui::DragFloat3("Look At",   &cp.lookAt.x,   0.1f);
    }

    if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (int i = 0; i < (int)scene.spheres.size(); ++i) {
            Sphere& s = scene.spheres[i];
            ImGui::PushID(i);
            if (ImGui::TreeNode(("Obj " + std::to_string(i)).c_str())) {
                changed |= ImGui::DragFloat3("Center", &s.center.x, 0.1f);
                changed |= ImGui::SliderFloat ("Radius", &s.radius,  0.05f, 100.0f); // Збільшив ліміт радіуса для стін
                
                const char* types[] = {"Lambertian","Metal","Dielectric"};
                int t = (int)s.mat.type;
                if (ImGui::Combo("Material", &t, types, 3)) {
                    s.mat.type = (MaterialType)t; changed = true;
                }
                changed |= ImGui::ColorEdit3("Albedo", &s.mat.albedo.x);
                if (s.mat.type == MaterialType::Metal)
                    changed |= ImGui::SliderFloat("Fuzz", &s.mat.fuzz, 0.0f, 1.0f);
                if (s.mat.type == MaterialType::Dielectric)
                    changed |= ImGui::SliderFloat("IOR",  &s.mat.ior,  1.0f, 3.0f);
                
                if (ImGui::Button("Remove")) {
                    scene.spheres.erase(scene.spheres.begin() + i);
                    ImGui::TreePop(); ImGui::PopID();
                    changed = true; 
                    if (renderer.isRendering) ImGui::EndDisabled();
                    ImGui::End();
                    return true;
                }
                ImGui::TreePop();
            }
            ImGui::PopID();
        }
        if (ImGui::Button("+ Add Sphere")) {
            Sphere ns;
            ns.center = {0, 0, 0};
            ns.radius = 0.5f;
            scene.spheres.push_back(ns);
            changed = true;
        }
    }

    if (renderer.isRendering) ImGui::EndDisabled();

    ImGui::End();
    return changed;
}

}