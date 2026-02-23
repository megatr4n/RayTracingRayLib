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

            Vec3 forward;
            forward.x = std::cos(yawRad) * std::cos(pitchRad);
            forward.y = std::sin(pitchRad);
            forward.z = std::sin(yawRad) * std::cos(pitchRad);
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
            if (rec.mat.tex != nullptr) {
                return rec.mat.tex->value(rec.u, rec.v, rec.p);
            }
            return rec.mat.albedo;
        }
        return Vec3(0.15f, 0.15f, 0.15f);
    }

    Vec3 Renderer::traceRay(const Ray &r, int depth)
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
                return (1.0f - t) * Vec3{1.0f, 1.0f, 1.0f} + t * Vec3{0.5f, 0.7f, 1.0f};
            }
            else
            {
                return {0, 0, 0};
            }
        }

        Ray scattered;
        Vec3 attenuation;

        Vec3 emitted = rec.mat.emitted(rec.u, rec.v, rec.p);

        Vec3 unit = normalize(r.direction);
        if (rec.mat.scatter(r, rec, attenuation, scattered))
        {
            return emitted + attenuation * traceRay(scattered, depth - 1);
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

        if (samplesDone >= settings.samplesTarget)
            return;

        tf::Taskflow taskflow;
        taskflow.for_each_index(0, H, 1, [&](int j)
                                {
            for (int i = 0; i < W; ++i) {
                float u = (i + randomFloat()) / (W - 1);
                float v = (j + randomFloat()) / (H - 1);
                
                Ray r = camera.getRay(u, v);
                accumBuffer[j * W + i] += traceRay(r, settings.maxBounces);
            } });

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
        if (scene == nullptr) return; 

        Camera3D cam = { 0 };
        cam.position = { camParams.lookFrom.x, camParams.lookFrom.y, camParams.lookFrom.z };
        
        float yawRad = camParams.yaw * (3.14159265f / 180.0f);
        float pitchRad = camParams.pitch * (3.14159265f / 180.0f);
        Vec3 forward;
        forward.x = std::cos(yawRad) * std::cos(pitchRad);
        forward.y = std::sin(pitchRad);
        forward.z = std::sin(yawRad) * std::cos(pitchRad);
        
        cam.target = { cam.position.x + forward.x, cam.position.y + forward.y, cam.position.z + forward.z };
        cam.up = { 0.0f, 1.0f, 0.0f };
        cam.fovy = camParams.vfov;
        cam.projection = CAMERA_PERSPECTIVE;

        BeginMode3D(cam);

        for (const auto& s : scene->spheres) {
            Color col = { 
                (unsigned char)(s.mat.albedo.x * 255.0f), 
                (unsigned char)(s.mat.albedo.y * 255.0f), 
                (unsigned char)(s.mat.albedo.z * 255.0f), 255 
            };
            Vector3 center = { s.center.x, s.center.y, s.center.z };
            Vector3 size = { s.radius * 2.0f, s.radius * 2.0f, s.radius * 2.0f };
            
            DrawSphere(center, s.radius, col);
            DrawSphereWires(center, s.radius * 1.01f, 16, 16, ColorAlpha(GREEN, 0.6f));
            DrawCubeWiresV(center, size, ColorAlpha(GREEN, 0.9f));
        }

        for (const auto& t : scene->triangles) {
            Color col = { 
                (unsigned char)(t.mat.albedo.x * 255.0f), 
                (unsigned char)(t.mat.albedo.y * 255.0f), 
                (unsigned char)(t.mat.albedo.z * 255.0f), 255 
            };
            
            Vector3 p1 = { t.v0.x, t.v0.y, t.v0.z };
            Vector3 p2 = { t.v1.x, t.v1.y, t.v1.z };
            Vector3 p3 = { t.v2.x, t.v2.y, t.v2.z };

            DrawTriangle3D(p1, p2, p3, col);
            DrawTriangle3D(p1, p3, p2, col);

            DrawLine3D(p1, p2, GREEN);
            DrawLine3D(p2, p3, GREEN);
            DrawLine3D(p3, p1, GREEN);
        }

        for (const auto& q : scene->quads) {
            rt::Vec3 normal = rt::normalize(rt::cross(q.u, q.v));
            rt::Vec3 lightDir = rt::normalize(rt::Vec3{0.5f, 1.0f, -0.8f});
            float diffuse = std::max(0.4f, rt::dot(normal, lightDir));

            Color col = { 
                (unsigned char)(q.mat.albedo.x * diffuse * 255.0f), 
                (unsigned char)(q.mat.albedo.y * diffuse * 255.0f), 
                (unsigned char)(q.mat.albedo.z * diffuse * 255.0f), 255 
            };

            Vector3 p1 = { q.Q.x, q.Q.y, q.Q.z };
            Vector3 p2 = { q.Q.x + q.u.x, q.Q.y + q.u.y, q.Q.z + q.u.z };
            Vector3 p3 = { q.Q.x + q.u.x + q.v.x, q.Q.y + q.u.y + q.v.y, q.Q.z + q.u.z + q.v.z };
            Vector3 p4 = { q.Q.x + q.v.x, q.Q.y + q.v.y, q.Q.z + q.v.z };

            DrawTriangle3D(p1, p2, p3, col);
            DrawTriangle3D(p1, p3, p4, col);
            DrawTriangle3D(p1, p3, p2, col);
            DrawTriangle3D(p1, p4, p3, col);

            DrawLine3D(p1, p2, GREEN);
            DrawLine3D(p2, p3, GREEN);
            DrawLine3D(p3, p4, GREEN);
            DrawLine3D(p4, p1, GREEN);
        }

        // --- 4. МЕШІ (ПІРАМІДИ ТА ІНШІ 3D МОДЕЛІ) ---
        for (const auto& m : scene->meshes) {
            Color col = { 
                (unsigned char)(m.mat.albedo.x * 255.0f), 
                (unsigned char)(m.mat.albedo.y * 255.0f), 
                (unsigned char)(m.mat.albedo.z * 255.0f), 255 
            };
            for (const auto& t : m.localTriangles) {
                Vector3 p1 = { t.v0.x + m.position.x, t.v0.y + m.position.y, t.v0.z + m.position.z };
                Vector3 p2 = { t.v1.x + m.position.x, t.v1.y + m.position.y, t.v1.z + m.position.z };
                Vector3 p3 = { t.v2.x + m.position.x, t.v2.y + m.position.y, t.v2.z + m.position.z };

                DrawTriangle3D(p1, p2, p3, col);
                DrawTriangle3D(p1, p3, p2, col);
                
                DrawLine3D(p1, p2, GREEN); 
                DrawLine3D(p2, p3, GREEN); 
                DrawLine3D(p3, p1, GREEN);
            }
        }

        EndMode3D();
    }
}