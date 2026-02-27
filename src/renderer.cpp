#include <imgui.h>
#include "renderer.h"
#include "random_utils.h"
#include <algorithm>
#include <cmath>
#include <raymath.h>
#include <rlgl.h>

namespace rt
{

    void calculateInitialAngles(RtCameraParams &p)
    {
        Vec3 dir = normalize(p.lookAt - p.lookFrom);
        p.pitch = std::asin(dir.y) * (180.0f / 3.14159265f);
        p.yaw = std::atan2(dir.z, dir.x) * (180.0f / 3.14159265f);
    }

    void Renderer::init(Scene *s)
    {
        scene = s;
        calculateInitialAngles(camParams);
        camera.init(camParams);

        isPreview = true;
        isRendering = false;

        resize(settings.width, settings.height);

        EnableCursor();
    }

    void Renderer::resize(int w, int h)
    {
        settings.width = w;
        settings.height = h;
        albedoBuffer.assign(w * h, {0, 0, 0});
        normalBuffer.assign(w * h, {0, 0, 0});
        accumBuffer.assign(w * h, {0, 0, 0});
        pixels.assign(w * h * 4, 0);

        if (outputTex.id)
            UnloadTexture(outputTex);
        Image img = GenImageColor(w, h, BLACK);
        outputTex = LoadTextureFromImage(img);

        UnloadImage(img);

        SetTextureFilter(outputTex, TEXTURE_FILTER_BILINEAR);

        reset();
    }

    void Renderer::reset()
    {
        samplesDone = 0;
        std::fill(accumBuffer.begin(), accumBuffer.end(), Vec3{0, 0, 0});
        std::fill(albedoBuffer.begin(), albedoBuffer.end(), Vec3{0, 0, 0});
        std::fill(normalBuffer.begin(), normalBuffer.end(), Vec3{0, 0, 0});
    }

    void Renderer::startRender()
    {
        isRendering = true;
        isPreview = false;
        reset();
        EnableCursor();
    }

    void Renderer::stopRender()
    {
        isRendering = false;
        isPreview = true;
    }

    void Renderer::update(float dt)
    {
        if (isRendering)
            return;

        if (!IsCursorHidden() && !ImGui::GetIO().WantCaptureMouse)
        {
            Camera3D cam = {0};
            cam.position = {camParams.lookFrom.x, camParams.lookFrom.y, camParams.lookFrom.z};
            float yawRad = camParams.yaw * (3.14159265f / 180.0f);
            float pitchRad = camParams.pitch * (3.14159265f / 180.0f);
            Vec3 forward = {std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad), std::sin(yawRad) * std::cos(pitchRad)};
            cam.target = {cam.position.x + forward.x, cam.position.y + forward.y, cam.position.z + forward.z};
            cam.up = {0.0f, 1.0f, 0.0f};
            cam.fovy = camParams.vfov;
            cam.projection = CAMERA_PERSPECTIVE;

            ::Ray raylibRay = GetMouseRay(GetMousePosition(), cam);
            rt::Ray r = {{raylibRay.position.x, raylibRay.position.y, raylibRay.position.z},
                         {raylibRay.direction.x, raylibRay.direction.y, raylibRay.direction.z}};

            auto getAxisT = [&](rt::Vec3 axisDir, rt::Vec3 origin)
            {
                rt::Vec3 w = r.origin - origin;
                float a = 1.0f;
                float b = rt::dot(r.direction, axisDir);
                float c = 1.0f;
                float d = rt::dot(r.direction, w);
                float e = rt::dot(axisDir, w);
                float den = a * c - b * b;
                if (std::abs(den) < 0.0001f)
                    return 0.0f;
                return (a * e - b * d) / den;
            };

            rt::Vec3 objPos = {0, 0, 0};
            bool hasObj = false;
            if (selection.type == ObjType::Sphere)
            {
                objPos = scene->spheres[selection.index].center;
                hasObj = true;
            }
            else if (selection.type == ObjType::Quad)
            {
                objPos = scene->quads[selection.index].Q + (scene->quads[selection.index].u + scene->quads[selection.index].v) * 0.5f;
                hasObj = true;
            }
            else if (selection.type == ObjType::Mesh)
            {
                objPos = scene->meshes[selection.index].position;
                hasObj = true;
            }

            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))
            {
                bool clickedGizmo = false;
                if (hasObj)
                {
                    float s = 0.1f;
                    float l = 1.5f;
                    BoundingBox boxX = {{objPos.x, objPos.y - s, objPos.z - s}, {objPos.x + l, objPos.y + s, objPos.z + s}};
                    BoundingBox boxY = {{objPos.x - s, objPos.y, objPos.z - s}, {objPos.x + s, objPos.y + l, objPos.z + s}};
                    BoundingBox boxZ = {{objPos.x - s, objPos.y - s, objPos.z}, {objPos.x + s, objPos.y + s, objPos.z + l}};

                    if (GetRayCollisionBox(raylibRay, boxX).hit)
                    {
                        draggingAxis = 0;
                        clickedGizmo = true;
                        initialDragT = getAxisT({1, 0, 0}, objPos);
                        initialObjPos = objPos;
                    }
                    else if (GetRayCollisionBox(raylibRay, boxY).hit)
                    {
                        draggingAxis = 1;
                        clickedGizmo = true;
                        initialDragT = getAxisT({0, 1, 0}, objPos);
                        initialObjPos = objPos;
                    }
                    else if (GetRayCollisionBox(raylibRay, boxZ).hit)
                    {
                        draggingAxis = 2;
                        clickedGizmo = true;
                        initialDragT = getAxisT({0, 0, 1}, objPos);
                        initialObjPos = objPos;
                    }
                }

                if (!clickedGizmo)
                {
                    float minT = 1e9f;
                    ObjType hitType = ObjType::None;
                    int hitIndex = -1;

                    for (int i = 0; i < (int)scene->spheres.size(); ++i)
                    {
                        HitRecord rec;
                        if (scene->spheres[i].hit(r, 0.001f, minT, rec))
                        {
                            minT = rec.t;
                            hitType = ObjType::Sphere;
                            hitIndex = i;
                        }
                    }
                    for (int i = 0; i < (int)scene->quads.size(); ++i)
                    {
                        HitRecord rec;
                        if (scene->quads[i].hit(r, 0.001f, minT, rec))
                        {
                            minT = rec.t;
                            hitType = ObjType::Quad;
                            hitIndex = i;
                        }
                    }
                    for (int i = 0; i < (int)scene->meshes.size(); ++i)
                    {
                        HitRecord rec;
                        if (scene->meshes[i].hit(r, 0.001f, minT, rec))
                        {
                            minT = rec.t;
                            hitType = ObjType::Mesh;
                            hitIndex = i;
                        }
                    }

                    selection = {hitType, hitIndex};
                    draggingAxis = -1;
                }
            }

            if (IsMouseButtonReleased(MOUSE_BUTTON_LEFT))
            {
                if (draggingAxis != -1)
                    scene->buildBVH();
                draggingAxis = -1;
            }

            if (draggingAxis != -1 && IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                rt::Vec3 axisDir = {0, 0, 0};
                if (draggingAxis == 0)
                    axisDir = {1, 0, 0};
                if (draggingAxis == 1)
                    axisDir = {0, 1, 0};
                if (draggingAxis == 2)
                    axisDir = {0, 0, 1};

                float currentT = getAxisT(axisDir, initialObjPos);
                rt::Vec3 newPos = initialObjPos + axisDir * (currentT - initialDragT);

                if (selection.type == ObjType::Sphere)
                    scene->spheres[selection.index].center = newPos;
                else if (selection.type == ObjType::Mesh)
                    scene->meshes[selection.index].position = newPos;
                else if (selection.type == ObjType::Quad)
                {
                    rt::Vec3 oldCenter = scene->quads[selection.index].Q + (scene->quads[selection.index].u + scene->quads[selection.index].v) * 0.5f;
                    scene->quads[selection.index].Q = scene->quads[selection.index].Q + (newPos - oldCenter);
                    scene->quads[selection.index].init(scene->quads[selection.index].Q, scene->quads[selection.index].u, scene->quads[selection.index].v, scene->quads[selection.index].mat);
                }
            }
        }

        if (IsKeyPressed(KEY_TAB))
        {
            if (IsCursorHidden())
                EnableCursor();
            else
                DisableCursor();
        }
        bool moved = false;
        if (IsCursorHidden())
        {
            Vector2 delta = GetMouseDelta();
            if (delta.x != 0 || delta.y != 0)
            {
                camParams.yaw += delta.x * settings.mouseSens;
                camParams.pitch -= delta.y * settings.mouseSens;
                if (camParams.pitch > 89.0f)
                    camParams.pitch = 89.0f;
                if (camParams.pitch < -89.0f)
                    camParams.pitch = -89.0f;
                moved = true;
            }
            float yawRad = camParams.yaw * (3.14159265f / 180.0f);
            float pitchRad = camParams.pitch * (3.14159265f / 180.0f);
            Vec3 forward = {std::cos(yawRad) * std::cos(pitchRad), std::sin(pitchRad), std::sin(yawRad) * std::cos(pitchRad)};
            forward = normalize(forward);
            Vec3 right = normalize(cross(forward, camParams.vUp));
            Vec3 velocity = {0, 0, 0};
            if (IsKeyDown(KEY_W))
                velocity += forward;
            if (IsKeyDown(KEY_S))
                velocity -= forward;
            if (IsKeyDown(KEY_D))
                velocity += right;
            if (IsKeyDown(KEY_A))
                velocity -= right;
            if (IsKeyDown(KEY_Q))
                velocity -= Vec3(0, 1, 0);
            if (IsKeyDown(KEY_E))
                velocity += Vec3(0, 1, 0);
            if (velocity.lengthSq() > 0)
            {
                camParams.lookFrom += normalize(velocity) * settings.moveSpeed * dt;
                moved = true;
            }
        }
        if (moved)
        {
            camera.updateViewMatrix(camParams);
            reset();
        }
    }

    Vec3 Renderer::tracePreview(const Ray &r)
    {
        HitRecord rec;
        if (scene->hit(r, 0.001f, 1e9f, rec))
        {
            if (rec.mat.tex != nullptr)
            {
                return rec.mat.tex->value(rec.u, rec.v, rec.p);
            }
            return rec.mat.albedo;
        }
        return Vec3(0.15f, 0.15f, 0.15f);
    }

    Vec3 Renderer::traceRay(const Ray &r, int depth, bool isPrimary, Vec3 &outAlbedo, Vec3 &outNormal)
    {
        if (depth <= 0)
            return {0, 0, 0};

        HitRecord rec;

        if (!scene->hit(r, 0.001f, 1e9f, rec))
        {
            if (scene->quads.empty())
            {
                Vec3 unit = normalize(r.direction);
                float t = 0.5f * (unit.y + 1.0f);
                Vec3 skyColor = (1.0f - t) * Vec3{1.0f, 1.0f, 1.0f} + t * Vec3{0.5f, 0.7f, 1.0f};

                if (isPrimary)
                {
                    outAlbedo = rec.mat.albedo;
                    outNormal = rec.normal;
                }
                return skyColor;
            }
            else
            {
                if (isPrimary)
                {
                    outAlbedo = {0, 0, 0};
                    outNormal = {0, 0, 0};
                }
                return {0, 0, 0};
            }
        }

        if (isPrimary)
        {
            outAlbedo = rec.mat.albedo;
            outNormal = (rec.normal + Vec3{1.0f, 1.0f, 1.0f}) * 0.5f;
        }

        Ray scattered;
        Vec3 attenuation;
        Vec3 emitted = rec.mat.emitted(rec.u, rec.v, rec.p);

        if (rec.mat.scatter(r, rec, attenuation, scattered))
        {
            return emitted + attenuation * traceRay(scattered, depth - 1, false, outAlbedo, outNormal);
        }
        return emitted;
    }

    void Renderer::renderSample()
    {
        int W = settings.width, H = settings.height;

        if (!isRendering)
        {
            return;
        }
        if (!accumulateRays)
            return;

        if (samplesDone >= settings.samplesTarget)
            return;

        tf::Taskflow taskflow;
        taskflow.for_each_index(0, H, 1, [&](int j)
                                {
            for (int i = 0; i < W; ++i) {
                float u = (i + randomFloat()) / (W - 1);
                float v = (j + randomFloat()) / (H - 1);
                
                Ray r = camera.getRay(u, v);
                Vec3 pixelAlbedo = {0,0,0};
                Vec3 pixelNormal = {0,0,0};
                Vec3 pixelColor = traceRay(r, settings.maxBounces, true, pixelAlbedo, pixelNormal); 
                
                int idx = j * W + i;
                accumBuffer[idx]  += pixelColor;
                albedoBuffer[idx] += pixelAlbedo;
                normalBuffer[idx] += pixelNormal;
            }
        });

        executor.run(taskflow).wait();

        samplesDone++;
        uploadPixels();
    }

    void Renderer::uploadPixels()
    {
        int W = settings.width, H = settings.height;
        for (int j = 0; j < H; ++j)
        {
            for (int i = 0; i < W; ++i)
            {
                int srcIdx = j * W + i;
                Color c = accumBuffer[srcIdx].toColor(samplesDone > 0 ? samplesDone : 1);

                int destIdx = (j * W + i) * 4;
                pixels[destIdx + 0] = c.r;
                pixels[destIdx + 1] = c.g;
                pixels[destIdx + 2] = c.b;
                pixels[destIdx + 3] = 255;
            }
        }
        UpdateTexture(outputTex, pixels.data());
    }

    void Renderer::drawRaylibPreview()
    {
        if (scene == nullptr)
            return;

        Camera3D cam = {0};
        cam.position = {camParams.lookFrom.x, camParams.lookFrom.y, camParams.lookFrom.z};

        float yawRad = camParams.yaw * (3.14159265f / 180.0f);
        float pitchRad = camParams.pitch * (3.14159265f / 180.0f);
        Vec3 forward;
        forward.x = std::cos(yawRad) * std::cos(pitchRad);
        forward.y = std::sin(pitchRad);
        forward.z = std::sin(yawRad) * std::cos(pitchRad);

        cam.target = {cam.position.x + forward.x, cam.position.y + forward.y, cam.position.z + forward.z};
        cam.up = {0.0f, 1.0f, 0.0f};
        cam.fovy = camParams.vfov;
        cam.projection = CAMERA_PERSPECTIVE;

        BeginMode3D(cam);

        DrawGrid(40, 1.0f);

        DrawLine3D({0.0f, 0.0f, 0.0f}, {5.0f, 0.0f, 0.0f}, RED);
        DrawLine3D({0.0f, 0.0f, 0.0f}, {0.0f, 5.0f, 0.0f}, GREEN);
        DrawLine3D({0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 5.0f}, BLUE);

        for (const auto &s : scene->spheres)
        {
            Color col = {
                (unsigned char)(s.mat.albedo.x * 255.0f),
                (unsigned char)(s.mat.albedo.y * 255.0f),
                (unsigned char)(s.mat.albedo.z * 255.0f), 255};
            Vector3 center = {s.center.x, s.center.y, s.center.z};
            Vector3 size = {s.radius * 2.0f, s.radius * 2.0f, s.radius * 2.0f};

            DrawSphere(center, s.radius, col);
            DrawSphereWires(center, s.radius * 1.01f, 16, 16, ColorAlpha(GREEN, 0.6f));
            DrawCubeWiresV(center, size, ColorAlpha(GREEN, 0.9f));
        }

        for (const auto &t : scene->triangles)
        {
            Color col = {
                (unsigned char)(t.mat.albedo.x * 255.0f),
                (unsigned char)(t.mat.albedo.y * 255.0f),
                (unsigned char)(t.mat.albedo.z * 255.0f), 255};

            Vector3 p1 = {t.v0.x, t.v0.y, t.v0.z};
            Vector3 p2 = {t.v1.x, t.v1.y, t.v1.z};
            Vector3 p3 = {t.v2.x, t.v2.y, t.v2.z};

            DrawTriangle3D(p1, p2, p3, col);
            DrawTriangle3D(p1, p3, p2, col);

            DrawLine3D(p1, p2, GREEN);
            DrawLine3D(p2, p3, GREEN);
            DrawLine3D(p3, p1, GREEN);
        }

        for (const auto &q : scene->quads)
        {
            rt::Vec3 normal = rt::normalize(rt::cross(q.u, q.v));
            rt::Vec3 lightDir = rt::normalize(rt::Vec3{0.5f, 1.0f, -0.8f});
            float diffuse = std::max(0.4f, rt::dot(normal, lightDir));

            Color col = {
                (unsigned char)(q.mat.albedo.x * diffuse * 255.0f),
                (unsigned char)(q.mat.albedo.y * diffuse * 255.0f),
                (unsigned char)(q.mat.albedo.z * diffuse * 255.0f), 255};

            Vector3 p1 = {q.Q.x, q.Q.y, q.Q.z};
            Vector3 p2 = {q.Q.x + q.u.x, q.Q.y + q.u.y, q.Q.z + q.u.z};
            Vector3 p3 = {q.Q.x + q.u.x + q.v.x, q.Q.y + q.u.y + q.v.y, q.Q.z + q.u.z + q.v.z};
            Vector3 p4 = {q.Q.x + q.v.x, q.Q.y + q.v.y, q.Q.z + q.v.z};

            DrawTriangle3D(p1, p2, p3, col);
            DrawTriangle3D(p1, p3, p4, col);
            DrawTriangle3D(p1, p3, p2, col);
            DrawTriangle3D(p1, p4, p3, col);

            DrawLine3D(p1, p2, GREEN);
            DrawLine3D(p2, p3, GREEN);
            DrawLine3D(p3, p4, GREEN);
            DrawLine3D(p4, p1, GREEN);
        }

        for (const auto &m : scene->meshes)
        {
            Color col = {
                (unsigned char)(m.mat.albedo.x * 255.0f),
                (unsigned char)(m.mat.albedo.y * 255.0f),
                (unsigned char)(m.mat.albedo.z * 255.0f), 255};
            for (const auto &t : m.localTriangles)
            {
                Vector3 p1 = {t.v0.x + m.position.x, t.v0.y + m.position.y, t.v0.z + m.position.z};
                Vector3 p2 = {t.v1.x + m.position.x, t.v1.y + m.position.y, t.v1.z + m.position.z};
                Vector3 p3 = {t.v2.x + m.position.x, t.v2.y + m.position.y, t.v2.z + m.position.z};

                DrawTriangle3D(p1, p2, p3, col);
                DrawTriangle3D(p1, p3, p2, col);

                DrawLine3D(p1, p2, GREEN);
                DrawLine3D(p2, p3, GREEN);
                DrawLine3D(p3, p1, GREEN);
            }
        }
        if (selection.type != ObjType::None && !isRendering)
        {
            Vector3 center = {0, 0, 0};
            if (selection.type == ObjType::Sphere)
            {
                center = {scene->spheres[selection.index].center.x, scene->spheres[selection.index].center.y, scene->spheres[selection.index].center.z};
            }
            else if (selection.type == ObjType::Quad)
            {
                rt::Vec3 c = scene->quads[selection.index].Q + (scene->quads[selection.index].u + scene->quads[selection.index].v) * 0.5f;
                center = {c.x, c.y, c.z};
            }
            else if (selection.type == ObjType::Mesh)
            {
                center = {scene->meshes[selection.index].position.x, scene->meshes[selection.index].position.y, scene->meshes[selection.index].position.z};
            }

            rlDisableDepthTest();

            float thick = 0.015f;
            float tip = 0.08f;
            float len = 1.2f;
            int segs = 16;
            DrawCylinderEx(center, {center.x + len, center.y, center.z}, thick, thick, segs, RED);
            DrawCylinderEx({center.x + len, center.y, center.z}, {center.x + len + 0.2f, center.y, center.z}, tip, 0.0f, segs, RED);
            DrawCylinderEx(center, {center.x, center.y + len, center.z}, thick, thick, segs, GREEN);
            DrawCylinderEx({center.x, center.y + len, center.z}, {center.x, center.y + len + 0.2f, center.z}, tip, 0.0f, segs, GREEN);
            DrawCylinderEx(center, {center.x, center.y, center.z + len}, thick, thick, segs, BLUE);
            DrawCylinderEx({center.x, center.y, center.z + len}, {center.x, center.y, center.z + len + 0.2f}, tip, 0.0f, segs, BLUE);

            rlEnableDepthTest();
        }
        EndMode3D();
    }

    void Renderer::saveRenderToPNG(const std::string &filename)
    {
        Image img;
        img.data = pixels.data();
        img.width = settings.width;
        img.height = settings.height;
        img.mipmaps = 1;
        img.format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8;

        ExportImage(img, filename.c_str());
    }

    void denoise();

    void Renderer::denoise()
    {
        if (samplesDone == 0)
            return;

        int W = settings.width;
        int H = settings.height;
        float divisor = (float)samplesDone;

        std::vector<float> colorFlat(W * H * 3);
        std::vector<float> albedoFlat(W * H * 3);
        std::vector<float> normalFlat(W * H * 3);

        for (int i = 0; i < W * H; ++i)
        {
            Vec3 c = accumBuffer[i] / divisor;
            Vec3 a = albedoBuffer[i] / divisor;
            Vec3 n = normalBuffer[i] / divisor;

            colorFlat[i * 3 + 0] = c.x;
            colorFlat[i * 3 + 1] = c.y;
            colorFlat[i * 3 + 2] = c.z;
            albedoFlat[i * 3 + 0] = a.x;
            albedoFlat[i * 3 + 1] = a.y;
            albedoFlat[i * 3 + 2] = a.z;
            normalFlat[i * 3 + 0] = n.x;
            normalFlat[i * 3 + 1] = n.y;
            normalFlat[i * 3 + 2] = n.z;
        }

        oidn::DeviceRef device = oidn::newDevice();
        device.commit();

        oidn::FilterRef filter = device.newFilter("RT");
        filter.setImage("color", colorFlat.data(), oidn::Format::Float3, W, H);
        filter.setImage("albedo", albedoFlat.data(), oidn::Format::Float3, W, H);
        filter.setImage("normal", normalFlat.data(), oidn::Format::Float3, W, H);
        filter.setImage("output", colorFlat.data(), oidn::Format::Float3, W, H);
        filter.set("hdr", true);
        filter.set("cleanAux", false);
        filter.commit();

        filter.execute();

        const char *errorMessage;
        if (device.getError(errorMessage) != oidn::Error::None)
        {
            printf("OIDN Error: %s\n", errorMessage);
        }
        for (int i = 0; i < W * H; ++i)
        {
            accumBuffer[i] = {colorFlat[i * 3 + 0], colorFlat[i * 3 + 1], colorFlat[i * 3 + 2]};
        }
        std::fill(albedoBuffer.begin(), albedoBuffer.end(), Vec3{0, 0, 0});
        std::fill(normalBuffer.begin(), normalBuffer.end(), Vec3{0, 0, 0});
        samplesDone = 1;
        uploadPixels();
    }
}