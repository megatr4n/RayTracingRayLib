#include "ui.h"
#include "renderer.h"
#include "random_utils.h"
#include "texture.h"
#include <imgui.h>
#include <string>
#include <vector>
#include <ctime>
#include <chrono>

namespace rt
{

    void createRandomScene(Scene &scene, RtCameraParams &cam)
    {
        scene.spheres.clear();
        scene.quads.clear();
        scene.triangles.clear();
        scene.meshes.clear();
        scene.materials.clear();

        scene.materials.push_back({MaterialType::Lambertian, {0.5f, 0.5f, 0.5f}});
        scene.spheres.push_back({{0, -1000, 0}, 1000, (uint32_t)(scene.materials.size() - 1)});

        scene.materials.push_back({MaterialType::Dielectric, {1.0f, 1.0f, 1.0f}, {0, 0, 0}, 0.0f, 1.5f});
        scene.spheres.push_back({{0, 1, 0}, 1, (uint32_t)(scene.materials.size() - 1)});

        scene.materials.push_back({MaterialType::Lambertian, {0.4f, 0.2f, 0.1f}});
        scene.spheres.push_back({{-4, 1, 0}, 1, (uint32_t)(scene.materials.size() - 1)});

        scene.materials.push_back({MaterialType::Metal, {0.7f, 0.6f, 0.5f}, {0, 0, 0}, 0.0f});
        scene.spheres.push_back({{4, 1, 0}, 1, (uint32_t)(scene.materials.size() - 1)});

        for (int a = -5; a < 5; a++)
        {
            for (int b = -5; b < 5; b++)
            {
                float choose_mat = randomFloat();
                Vec3 center = {a + 0.9f * randomFloat(), 0.2f, b + 0.9f * randomFloat()};
                if ((center - Vec3{4, 0.2f, 0}).length() > 0.9f)
                {
                    Sphere s;
                    s.center = center;
                    s.radius = 0.2f;

                    Material newMat;
                    if (choose_mat < 0.8f)
                    {
                        newMat.type = MaterialType::Lambertian;
                        newMat.albedo = randomVec3() * randomVec3();
                    }
                    else if (choose_mat < 0.95f)
                    {
                        newMat.type = MaterialType::Metal;
                        newMat.albedo = randomVec3(0.5f, 1.0f);
                        newMat.fuzz = randomFloat(0.0f, 0.5f);
                    }
                    else
                    {
                        newMat.type = MaterialType::Dielectric;
                        newMat.ior = 1.5f;
                    }
                    scene.materials.push_back(newMat);
                    s.matIndex = scene.materials.size() - 1;

                    scene.spheres.push_back(s);
                }
            }
        }

        cam.lookFrom = {13, 2, 3};
        cam.lookAt = {0, 0, 0};
        cam.vfov = 20.0f;
        cam.aperture = 0.0f;
        cam.focusDist = 10.0f;

        Vec3 dir = normalize(cam.lookAt - cam.lookFrom);
        cam.pitch = std::asin(dir.y) * (180.0f / 3.14159265f);
        cam.yaw = std::atan2(dir.z, dir.x) * (180.0f / 3.14159265f);
    }

    void addBox(Scene &scene, Vec3 p0, Vec3 p1, uint32_t matIndex, float angleY = 0.0f)
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
            q.init(Q, u, v, matIndex);
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
        scene.triangles.clear();
        scene.meshes.clear();
        scene.materials.clear();

        scene.materials.push_back({MaterialType::Lambertian, {0.65f, 0.05f, 0.05f}});
        uint32_t red = scene.materials.size() - 1;

        scene.materials.push_back({MaterialType::Lambertian, {0.73f, 0.73f, 0.73f}});
        uint32_t white = scene.materials.size() - 1;

        scene.materials.push_back({MaterialType::Lambertian, {0.12f, 0.45f, 0.15f}});
        uint32_t green = scene.materials.size() - 1;

        scene.materials.push_back({MaterialType::DiffuseLight, {0, 0, 0}, {15, 15, 15}});
        uint32_t light = scene.materials.size() - 1;

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

        scene.materials.push_back({MaterialType::Dielectric, {1.0f, 1.0f, 1.0f}, {0, 0, 0}, 0.0f, 1.5f});
        scene.spheres.push_back({{190, 90, 190}, 90, (uint32_t)(scene.materials.size() - 1)});

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

        static bool styleSet = false;
        if (!styleSet)
        {
            ImGuiStyle &style = ImGui::GetStyle();

            style.WindowRounding = 8.0f;
            style.FrameRounding = 5.0f;
            style.GrabRounding = 5.0f;
            style.PopupRounding = 5.0f;
            style.ScrollbarRounding = 12.0f;
            style.ItemSpacing = ImVec2(8, 6);

            ImVec4 *colors = style.Colors;
            colors[ImGuiCol_Text] = ImVec4(0.90f, 0.90f, 0.90f, 1.00f);
            colors[ImGuiCol_WindowBg] = ImVec4(0.12f, 0.12f, 0.12f, 0.98f);
            colors[ImGuiCol_TitleBg] = ImVec4(0.08f, 0.08f, 0.08f, 1.00f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
            colors[ImGuiCol_FrameBg] = ImVec4(0.20f, 0.20f, 0.20f, 1.00f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.30f, 0.30f, 0.30f, 1.00f);
            colors[ImGuiCol_Button] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(0.28f, 0.32f, 0.36f, 1.00f);
            colors[ImGuiCol_ButtonActive] = ImVec4(0.15f, 0.18f, 0.22f, 1.00f);
            colors[ImGuiCol_Header] = ImVec4(0.22f, 0.22f, 0.22f, 1.00f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.28f, 0.32f, 0.36f, 1.00f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.32f, 0.36f, 0.40f, 1.00f);
            colors[ImGuiCol_SliderGrab] = ImVec4(0.45f, 0.45f, 0.45f, 1.00f);
            colors[ImGuiCol_SliderGrabActive] = ImVec4(0.55f, 0.55f, 0.55f, 1.00f);
            colors[ImGuiCol_Separator] = ImVec4(0.25f, 0.25f, 0.25f, 1.00f);

            styleSet = true;
        }

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
                renderer.accumulateRays = true;
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

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.7f, 0.6f));
        if (ImGui::Button("Denoise Image (AI)", ImVec2(-1, 30)))
        {
            renderer.denoise();
            renderer.accumulateRays = false;
        }
        ImGui::PopStyleColor();

        ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.33f, 0.7f, 0.6f));
        if (ImGui::Button("Save Render to PNG", ImVec2(-1, 30)))
        {
            std::string filename = "render_" + std::to_string(time(nullptr)) + ".png";
            renderer.saveRenderToPNG(filename);
        }
        ImGui::PopStyleColor();

        ImGui::Separator();

        if (renderer.isRendering)
            ImGui::BeginDisabled();

        if (ImGui::CollapsingHeader("Performance Metrics", ImGuiTreeNodeFlags_DefaultOpen))
        {
            ImGui::Text("BVH Build Time:  %.2f ms", scene.lastBvhBuildTimeMs);
            ImGui::Text("Last Frame Time: %.2f ms", renderer.lastFrameTimeMs);

            ImGui::TextColored(ImVec4(0.4f, 1.0f, 0.4f, 1.0f), "Performance:     %.2f MRays/s", renderer.mraysPerSecond);

            ImGui::Separator();
            ImGui::Text("Threads used: %ld (Taskflow)", renderer.executor.num_workers());
        }

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

                bool isSelected = (renderer.selection.type == ObjType::Sphere && renderer.selection.index == i);
                if (isSelected)
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                }

                bool nodeOpen = ImGui::TreeNode(("Obj " + std::to_string(i)).c_str());

                if (isSelected)
                    ImGui::PopStyleColor();

                if (nodeOpen)
                {
                    changed |= ImGui::DragFloat3("Center", &s.center.x, 0.1f);
                    changed |= ImGui::SliderFloat("Radius", &s.radius, 0.05f, 100.0f);

                    Material &objMat = scene.materials[s.matIndex];

                    const char *types[] = {"Lambertian", "Metal", "Dielectric", "DiffuseLight"};
                    int t = (int)objMat.type;
                    if (ImGui::Combo("Material", &t, types, 4))
                    {
                        objMat.type = (MaterialType)t;
                        changed = true;
                    }
                    changed |= ImGui::ColorEdit3("Albedo", &objMat.albedo.x);
                    if (objMat.type == MaterialType::Metal)
                        changed |= ImGui::SliderFloat("Fuzz", &objMat.fuzz, 0.0f, 1.0f);
                    if (objMat.type == MaterialType::Dielectric)
                        changed |= ImGui::SliderFloat("IOR", &objMat.ior, 1.0f, 3.0f);

                    ImGui::Separator();
                    ImGui::Text("Textures:");
                    if (ImGui::Button("Apply Checkerboard"))
                    {
                        objMat.tex = std::make_shared<rt::CheckerTexture>(
                            2.0f, rt::Vec3(0.2f, 0.3f, 0.1f), rt::Vec3(0.9f, 0.9f, 0.9f));
                        changed = true;
                    }
                    if (ImGui::Button("Apply 'earth.jpg'"))
                    {
                        objMat.tex = std::make_shared<rt::ImageTexture>("assets/earth.jpg");
                        changed = true;
                    }
                    if (ImGui::Button("Clear Texture"))
                    {
                        objMat.tex = nullptr;
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

            for (int i = 0; i < (int)scene.quads.size(); ++i)
            {
                Quad &q = scene.quads[i];
                ImGui::PushID(1000 + i);

                bool isSelected = (renderer.selection.type == ObjType::Quad && renderer.selection.index == i);
                if (isSelected)
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                }

                bool nodeOpen = ImGui::TreeNode(("Quad " + std::to_string(i)).c_str());

                if (isSelected)
                    ImGui::PopStyleColor();

                if (nodeOpen)
                {
                    Vec3 center = q.Q + (q.u + q.v) * 0.5f;
                    Vec3 oldCenter = center;
                    if (ImGui::DragFloat3("Move (Center)", &center.x, 0.1f))
                    {
                        Vec3 delta = center - oldCenter;
                        q.Q = q.Q + delta;
                        q.init(q.Q, q.u, q.v, q.matIndex);
                        changed = true;
                    }
                    ImGui::Separator();

                    bool quadMoved = false;
                    quadMoved |= ImGui::DragFloat3("Q (Origin)", &q.Q.x, 0.1f);
                    quadMoved |= ImGui::DragFloat3("u (Edge 1)", &q.u.x, 0.1f);
                    quadMoved |= ImGui::DragFloat3("v (Edge 2)", &q.v.x, 0.1f);

                    if (quadMoved)
                    {
                        q.init(q.Q, q.u, q.v, q.matIndex);
                        ;
                        changed = true;
                    }

                    Material &objMat = scene.materials[q.matIndex];
                    const char *types[] = {"Lambertian", "Metal", "Dielectric", "DiffuseLight"};
                    int t = (int)objMat.type;
                    if (ImGui::Combo("Material", &t, types, 4))
                    {
                        objMat.type = (MaterialType)t;
                        q.init(q.Q, q.u, q.v, q.matIndex);
                        changed = true;
                    }
                    if (ImGui::ColorEdit3("Albedo", &objMat.albedo.x))
                    {
                        q.init(q.Q, q.u, q.v, q.matIndex);
                        changed = true;
                    }

                    if (objMat.type == MaterialType::Metal)
                        changed |= ImGui::SliderFloat("Fuzz", &objMat.fuzz, 0.0f, 1.0f);
                    if (objMat.type == MaterialType::Dielectric)
                        changed |= ImGui::SliderFloat("IOR", &objMat.ior, 1.0f, 3.0f);

                    if (ImGui::Button("Remove"))
                    {
                        scene.quads.erase(scene.quads.begin() + i);
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

            for (int i = 0; i < (int)scene.meshes.size(); ++i)
            {
                Mesh &m = scene.meshes[i];
                ImGui::PushID(3000 + i);

                bool isSelected = (renderer.selection.type == ObjType::Mesh && renderer.selection.index == i);
                if (isSelected)
                {
                    ImGui::SetNextItemOpen(true, ImGuiCond_Always);
                    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.2f, 1.0f));
                }

                bool nodeOpen = ImGui::TreeNode(("Mesh " + std::to_string(i)).c_str());

                if (isSelected)
                    ImGui::PopStyleColor();

                if (nodeOpen)
                {
                    changed |= ImGui::DragFloat3("Position", &m.position.x, 0.1f);

                    Material &objMat = scene.materials[m.matIndex];
                    const char *types[] = {"Lambertian", "Metal", "Dielectric", "DiffuseLight"};
                    int t = (int)objMat.type;
                    if (ImGui::Combo("Material", &t, types, 4))
                    {
                        objMat.type = (MaterialType)t;
                        changed = true;
                    }
                    changed |= ImGui::ColorEdit3("Albedo", &objMat.albedo.x);

                    if (objMat.type == MaterialType::Metal)
                        changed |= ImGui::SliderFloat("Fuzz", &objMat.fuzz, 0.0f, 1.0f);
                    if (objMat.type == MaterialType::Dielectric)
                        changed |= ImGui::SliderFloat("IOR", &objMat.ior, 1.0f, 3.0f);

                    if (ImGui::Button("Remove"))
                    {
                        scene.meshes.erase(scene.meshes.begin() + i);
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

            ImGui::Separator();
            ImGui::Text("Add Primitives:");

            if (ImGui::Button("+ Sphere"))
            {
                Sphere ns;
                ns.center = {0, 0, 0};
                ns.radius = 0.5f;
                ns.matIndex = scene.materials.size() - 1;
                scene.spheres.push_back(ns);
                changed = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("+ Quad"))
            {
                Quad nq;
                nq.init({-1, 0, -1}, {2, 0, 0}, {0, 0, 2}, scene.materials.size() - 1);
                scene.quads.push_back(nq);
                changed = true;
            }

            ImGui::SameLine();

            if (ImGui::Button("+ Pyramid"))
            {
                scene.materials.push_back({MaterialType::Lambertian, {0.8f, 0.6f, 0.2f}});
                uint32_t pyrMat = scene.materials.size() - 1;

                scene.materials.push_back({MaterialType::Lambertian, {1.0f, 1.0f, 1.0f}});
                uint32_t dummy = scene.materials.size() - 1;

                Mesh pyr;
                pyr.position = {0, 0, 0};
                pyr.matIndex = pyrMat;

                Vec3 top = {0, 2, 0};
                Vec3 fl = {-1, 0, 1};
                Vec3 fr = {1, 0, 1};
                Vec3 bl = {-1, 0, -1};
                Vec3 br = {1, 0, -1};

                pyr.localTriangles.push_back({fl, fr, top, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, dummy});
                pyr.localTriangles.push_back({fr, br, top, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, dummy});
                pyr.localTriangles.push_back({br, bl, top, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, dummy});
                pyr.localTriangles.push_back({bl, fl, top, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, dummy});
                pyr.localTriangles.push_back({fl, bl, fr, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, dummy});
                pyr.localTriangles.push_back({fr, bl, br, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, false, dummy});

                scene.meshes.push_back(pyr);
                changed = true;
            }
            ImGui::SameLine();

            if (ImGui::Button("+ Load OBJ"))
            {
                Mesh loadedModel;

                Material modelMat;
                modelMat.type = MaterialType::Metal;
                modelMat.albedo = {0.8f, 0.6f, 0.2f};
                modelMat.fuzz = 0.1f;

                scene.materials.push_back(modelMat);

                uint32_t modelMatIdx = scene.materials.size() - 1;

                if (loadMeshFromOBJ("assets/white_oak.obj", loadedModel, modelMatIdx, {0, 1, 0}, 0.01f))
                {
                    loadedModel.buildBVH();
                    scene.meshes.push_back(loadedModel);
                    changed = true;
                }
                else
                {
                    std::cout << "Error: Failed to load white_oak.obj. Please check if the file is located in the same directory as the executable." << std::endl;
                }
            }
        }
        if (renderer.isRendering)
            ImGui::EndDisabled();
        ImGui::End();
        if (changed)
        {
            auto start = std::chrono::high_resolution_clock::now();
            scene.buildBVH();
            auto end = std::chrono::high_resolution_clock::now();
            renderer.lastBvhBuildTimeMs = std::chrono::duration<float, std::milli>(end - start).count();
        }
        return changed;
    }
}