#include "ui.h"
#include "renderer.h"
#include "random_utils.h"
#include "texture.h"
#include <imgui.h>
#include <string>
#include <vector>

namespace rt
{

    void createRandomScene(Scene &scene, RtCameraParams &cam)
    {
        scene.spheres.clear();
        scene.quads.clear();

        scene.spheres.push_back({{0, -1000, 0}, 1000, {MaterialType::Lambertian, {0.5f, 0.5f, 0.5f}}});

        scene.spheres.push_back({{0, 1, 0}, 1, {MaterialType::Dielectric, {1.0f, 1.0f, 1.0f}, {0, 0, 0}, 0.0f, 1.5f}});
        scene.spheres.push_back({{-4, 1, 0}, 1, {MaterialType::Lambertian, {0.4f, 0.2f, 0.1f}}});
        scene.spheres.push_back({{4, 1, 0}, 1, {MaterialType::Metal, {0.7f, 0.6f, 0.5f}, {0, 0, 0}, 0.0f}});

        for (int a = -11; a < 11; a++)
        {
            for (int b = -11; b < 11; b++)
            {
                float choose_mat = randomFloat();
                Vec3 center = {a + 0.9f * randomFloat(), 0.2f, b + 0.9f * randomFloat()};
                if ((center - Vec3{4, 0.2f, 0}).length() > 0.9f)
                {
                    Sphere s;
                    s.center = center;
                    s.radius = 0.2f;
                    if (choose_mat < 0.8f)
                    {
                        s.mat.type = MaterialType::Lambertian;
                        s.mat.albedo = randomVec3() * randomVec3();
                    }
                    else if (choose_mat < 0.95f)
                    {
                        s.mat.type = MaterialType::Metal;
                        s.mat.albedo = randomVec3(0.5f, 1.0f);
                        s.mat.fuzz = randomFloat(0.0f, 0.5f);
                    }
                    else
                    {
                        s.mat.type = MaterialType::Dielectric;
                        s.mat.ior = 1.5f;
                    }
                    scene.spheres.push_back(s);
                }
            }
        }

        cam.lookFrom = {13, 2, 3};
        cam.lookAt = {0, 0, 0};
        cam.vfov = 20.0f;
        cam.aperture = 0.0f;
        cam.focusDist = 10.0f;
    }

    void addBox(Scene &scene, Vec3 p0, Vec3 p1, Material mat, float angleY = 0.0f)
    {
        Vec3 min = {std::fmin(p0.x, p1.x), std::fmin(p0.y, p1.y), std::fmin(p0.z, p1.z)};
        Vec3 max = {std::fmax(p0.x, p1.x), std::fmax(p0.y, p1.y), std::fmax(p0.z, p1.z)};

        Vec3 center = (min + max) * 0.5f;
        Vec3 dx = {max.x - min.x, 0, 0};
        Vec3 dy = {0, max.y - min.y, 0};
        Vec3 dz = {0, 0, max.z - min.z};

        auto addRotatedQuad = [&](Vec3 Q, Vec3 u, Vec3 v)
        {
            if (angleY != 0.0f)
            {
                Q = (Q - center).rotateY(angleY) + center;
                u = u.rotateY(angleY);
                v = v.rotateY(angleY);
            }
            Quad q;
            q.init(Q, u, v, mat);
            scene.quads.push_back(q);
        };

        addRotatedQuad({min.x, min.y, max.z}, dx, dy);
        addRotatedQuad({max.x, min.y, min.z}, -dx, dy);
        addRotatedQuad({min.x, max.y, max.z}, dx, -dz);
        addRotatedQuad({min.x, min.y, min.z}, dx, dz);
        addRotatedQuad({max.x, min.y, max.z}, -dz, dy);
        addRotatedQuad({min.x, min.y, min.z}, dz, dy);
    }

    void createCornellBox(Scene &scene, RtCameraParams &cam, float angle1 = 15.0f, float angle2 = -18.0f)
    {
        scene.spheres.clear();
        scene.quads.clear();

        Material red = {MaterialType::Lambertian, {0.65f, 0.05f, 0.05f}};
        Material white = {MaterialType::Lambertian, {0.73f, 0.73f, 0.73f}};
        Material green = {MaterialType::Lambertian, {0.12f, 0.45f, 0.15f}};
        Material light = {MaterialType::DiffuseLight, {0, 0, 0}, {15, 15, 15}};

        Quad q;
        q.init({555, 0, 0}, {0, 555, 0}, {0, 0, 555}, green);
        scene.quads.push_back(q);
        q.init({0, 0, 0}, {0, 0, 555}, {0, 555, 0}, red);
        scene.quads.push_back(q);
        q.init({0, 0, 0}, {555, 0, 0}, {0, 0, 555}, white);
        scene.quads.push_back(q);
        q.init({555, 555, 555}, {-555, 0, 0}, {0, 0, -555}, white);
        scene.quads.push_back(q);
        q.init({0, 0, 555}, {555, 0, 0}, {0, 555, 0}, white);
        scene.quads.push_back(q);

        q.init({343, 554, 332}, {-130, 0, 0}, {0, 0, -105}, light);
        scene.quads.push_back(q);

        addBox(scene, {265, 0, 295}, {430, 330, 460}, white, angle1);
        addBox(scene, {130, 0, 65}, {295, 165, 230}, white, angle2);
        scene.spheres.push_back({{190, 90, 190}, 90, {MaterialType::Dielectric, {1.0f, 1.0f, 1.0f}, {0, 0, 0}, 0.0f, 1.5f}});

        cam.lookFrom = {278, 278, -800};
        cam.lookAt = {278, 278, 0};
        cam.vfov = 40.0f;
        cam.aperture = 0.0f;
        cam.focusDist = 10.0f;
        cam.yaw = 90.0f;
        cam.pitch = 0.0f;
    }

    bool drawUI(Renderer &renderer, Scene &scene)
    {
        bool changed = false;

        ImGui::SetNextWindowPos({10, 10}, ImGuiCond_Once);
        ImGui::SetNextWindowSize({340, 650}, ImGuiCond_Once);

        ImGui::Begin("Raytracer Controls");

        if (renderer.isRendering)
        {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.0f, 0.6f, 0.6f));
            if (ImGui::Button("STOP RENDER", ImVec2(-1, 30)))
            {
                renderer.stopRender();
            }
            ImGui::PopStyleColor();
        }
        else
        {
            ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.6f, 0.6f));
            if (ImGui::Button("START RENDER", ImVec2(-1, 30)))
            {
                renderer.startRender();
            }
            ImGui::PopStyleColor();
        }
        ImGui::Separator();

        if (!renderer.isRendering)
        {
            ImGui::Text("Switch Scene:");
            if (ImGui::Button("Random Spheres", ImVec2(150, 0)))
            {
                createRandomScene(scene, renderer.camParams);
                scene.buildBVH();
                renderer.camera.init(renderer.camParams);
                renderer.reset();
            }
            ImGui::SameLine();
            if (ImGui::Button("Cornell Box", ImVec2(150, 0)))
            {
                createCornellBox(scene, renderer.camParams);
                scene.buildBVH();
                renderer.camera.init(renderer.camParams);
                renderer.reset();
            }

            ImGui::Separator();
            ImGui::Text("Cornell Box Rotation:");

            static float box1Angle = 15.0f;
            static float box2Angle = -18.0f;
            bool rebuildBox = false;

            if (ImGui::SliderFloat("Tall Box Angle", &box1Angle, -180.0f, 180.0f))
                rebuildBox = true;
            if (ImGui::SliderFloat("Short Box Angle", &box2Angle, -180.0f, 180.0f))
                rebuildBox = true;

            if (rebuildBox)
            {
                createCornellBox(scene, renderer.camParams, box1Angle, box2Angle);
                scene.buildBVH();
                renderer.reset();
            }

            ImGui::Separator();
        }

        float progress = (float)renderer.samplesDone / renderer.settings.samplesTarget;
        ImGui::TextColored({0.4f, 1.0f, 0.4f, 1.0f}, "Samples: %d / %d",
                           renderer.samplesDone, renderer.settings.samplesTarget);
        ImGui::ProgressBar(progress, {-1, 0});
        ImGui::Separator();

        if (renderer.isRendering)
            ImGui::BeginDisabled();

        if (ImGui::CollapsingHeader("Render Settings", ImGuiTreeNodeFlags_DefaultOpen))
        {
            changed |= ImGui::SliderInt("Max Bounces", &renderer.settings.maxBounces, 1, 32);
            changed |= ImGui::SliderInt("Target Samples", &renderer.settings.samplesTarget, 1, 2048);
        }

        if (ImGui::CollapsingHeader("Camera", ImGuiTreeNodeFlags_DefaultOpen))
        {
            RtCameraParams &cp = renderer.camParams;
            changed |= ImGui::SliderFloat("FOV", &cp.vfov, 5.0f, 120.0f);
            changed |= ImGui::SliderFloat("Aperture", &cp.aperture, 0.0f, 2.0f);
            changed |= ImGui::SliderFloat("Focus Dist", &cp.focusDist, 0.5f, 50.0f);
            changed |= ImGui::DragFloat3("Look From", &cp.lookFrom.x, 0.1f);
            changed |= ImGui::DragFloat3("Look At", &cp.lookAt.x, 0.1f);
        }

        if (ImGui::CollapsingHeader("Scene Objects", ImGuiTreeNodeFlags_DefaultOpen))
        {
            for (int i = 0; i < (int)scene.spheres.size(); ++i)
            {
                Sphere &s = scene.spheres[i];
                ImGui::PushID(i);
                if (ImGui::TreeNode(("Obj " + std::to_string(i)).c_str()))
                {
                    changed |= ImGui::DragFloat3("Center", &s.center.x, 0.1f);
                    changed |= ImGui::SliderFloat("Radius", &s.radius, 0.05f, 100.0f);

                    const char *types[] = {"Lambertian", "Metal", "Dielectric"};
                    int t = (int)s.mat.type;
                    if (ImGui::Combo("Material", &t, types, 3))
                    {
                        s.mat.type = (MaterialType)t;
                        changed = true;
                    }
                    changed |= ImGui::ColorEdit3("Albedo", &s.mat.albedo.x);
                    if (s.mat.type == MaterialType::Metal)
                        changed |= ImGui::SliderFloat("Fuzz", &s.mat.fuzz, 0.0f, 1.0f);
                    if (s.mat.type == MaterialType::Dielectric)
                        changed |= ImGui::SliderFloat("IOR", &s.mat.ior, 1.0f, 3.0f);

                    ImGui::Separator();
                    bool useChecker = (s.mat.tex != nullptr);
                    if (ImGui::Checkbox("Checkerboard Texture", &useChecker))
                    {
                        if (useChecker)
                        {
                            s.mat.tex = std::make_shared<rt::CheckerTexture>(
                                2.0f,
                                rt::Vec3(0.2f, 0.3f, 0.1f),
                                rt::Vec3(0.9f, 0.9f, 0.9f));
                        }
                        else
                        {
                            s.mat.tex = nullptr;
                        }
                        changed = true;
                    }
                    ImGui::Separator();

                    if (ImGui::Button("Remove"))
                    {
                        scene.spheres.erase(scene.spheres.begin() + i);
                        ImGui::TreePop();
                        ImGui::PopID();
                        changed = true;
                        if (renderer.isRendering)
                            ImGui::EndDisabled();
                        ImGui::End();
                        return true;
                    }
                    ImGui::TreePop();
                }
                ImGui::PopID();
            }
            if (ImGui::Button("+ Add Sphere"))
            {
                Sphere ns;
                ns.center = {0, 0, 0};
                ns.radius = 0.5f;
                scene.spheres.push_back(ns);
                changed = true;
            }
        }

        if (renderer.isRendering)
            ImGui::EndDisabled();

        ImGui::End();

        if (changed)
        {
            scene.buildBVH();
        }
        return changed;
    }

}