#include "ui.h"
#include "random_utils.h"
#include <imgui.h>
#include <string>

bool drawUI(Renderer& renderer, Scene& scene) {
    bool changed = false;

    ImGui::SetNextWindowPos ({10, 10},   ImGuiCond_Once);
    ImGui::SetNextWindowSize({320, 600}, ImGuiCond_Once);
    ImGui::Begin("Raytracer Controls");

    float progress = (float)renderer.samplesDone / renderer.settings.samplesTarget;
    ImGui::TextColored({0.4f, 1.0f, 0.4f, 1.0f},
                       "Samples: %d / %d",
                       renderer.samplesDone,
                       renderer.settings.samplesTarget);
    ImGui::ProgressBar(progress, {-1, 0});
    ImGui::Separator();

    if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen)) {
        changed |= ImGui::SliderInt("Max Bounces",
                                    &renderer.settings.maxBounces, 1, 32);
        changed |= ImGui::SliderInt("Target Samples",
                                    &renderer.settings.samplesTarget, 1, 1024);
    }

    if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen)) {
        CameraParams& cp = renderer.camParams;
        changed |= ImGui::SliderFloat ("FOV",       &cp.vfov,       5.0f, 120.0f);
        changed |= ImGui::SliderFloat ("Aperture",  &cp.aperture,   0.0f, 2.0f);
        changed |= ImGui::SliderFloat ("Focus Dist",&cp.focusDist,  0.5f, 30.0f);
        changed |= ImGui::SliderFloat3("Look From", &cp.lookFrom.x, -20,  20);
        changed |= ImGui::SliderFloat3("Look At",   &cp.lookAt.x,   -10,  10);
    }

    if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen)) {
        for (int i = 0; i < (int)scene.spheres.size(); ++i) {
            Sphere& s = scene.spheres[i];
            ImGui::PushID(i);

            if (ImGui::TreeNode(("Sphere " + std::to_string(i)).c_str())) {
                changed |= ImGui::SliderFloat3("Center", &s.center.x, -10, 10);
                changed |= ImGui::SliderFloat ("Radius", &s.radius, 0.05f, 5.0f);

                const char* types[] = { "Lambertian", "Metal", "Dielectric" };
                int t = (int)s.mat.type;
                if (ImGui::Combo("Material", &t, types, 3)) {
                    s.mat.type = (MaterialType)t;
                    changed    = true;
                }

                changed |= ImGui::ColorEdit3("Albedo", &s.mat.albedo.x);

                if (s.mat.type == MaterialType::Metal)
                    changed |= ImGui::SliderFloat("Fuzz", &s.mat.fuzz, 0.0f, 1.0f);

                if (s.mat.type == MaterialType::Dielectric)
                    changed |= ImGui::SliderFloat("IOR",  &s.mat.ior,  1.0f, 3.0f);

                if (ImGui::Button("Remove")) {
                    scene.spheres.erase(scene.spheres.begin() + i);
                    ImGui::TreePop();
                    ImGui::PopID();
                    changed = true;
                    break;
                }

                ImGui::TreePop();
            }

            ImGui::PopID();
        }

        if (ImGui::Button("+ Add Sphere")) {
            Sphere ns;
            ns.center = { randomFloat(-3, 3), 0.5f, randomFloat(-3, 3) };
            ns.radius = 0.5f;
            scene.spheres.push_back(ns);
            changed = true;
        }
    }

    ImGui::End();
    return changed;
}