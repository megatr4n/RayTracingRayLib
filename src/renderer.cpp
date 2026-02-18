#include "renderer.h"
#include "random_utils.h"
#include <algorithm>
#include <cmath>
#include <raymath.h>

namespace rt {

// Функція для правильного розрахунку кутів при старті
// Щоб камера не "стрибала", коли ти торкаєшся миші
void calculateInitialAngles(RtCameraParams& p) {
    Vec3 dir = normalize(p.lookAt - p.lookFrom);
    
    // Pitch (вгору-вниз)
    p.pitch = std::asin(dir.y) * (180.0f / 3.14159265f);
    
    // Yaw (ліво-право)
    // atan2 повертає кут у радіанах, переводимо в градуси
    p.yaw = std::atan2(dir.z, dir.x) * (180.0f / 3.14159265f);
}

void Renderer::init(Scene* s) {
    scene = s;
    
    // === ВИПРАВЛЕННЯ КАМЕРИ ===
    // Рахуємо правильні кути перед стартом
    calculateInitialAngles(camParams);
    
    camera.init(camParams);
    
    // Старт: режим польоту (чистий колір), рендер вимкнений
    isPreview = true;
    isRendering = false;
    
    resize(settings.width, settings.height);
    
    EnableCursor();
}

void Renderer::resize(int w, int h) {
    settings.width  = w;
    settings.height = h;
    accumBuffer.assign(w * h, {0, 0, 0});
    pixels.assign(w * h * 4, 0);
    
    if (outputTex.id) UnloadTexture(outputTex);
    Image img = GenImageColor(w, h, BLACK);
    outputTex = LoadTextureFromImage(img);
    UnloadImage(img);
    
    reset();
}

void Renderer::reset() {
    samplesDone = 0;
    std::fill(accumBuffer.begin(), accumBuffer.end(), Vec3{0, 0, 0});
}

void Renderer::startRender() {
    isRendering = true;
    isPreview = false;
    reset();           
    EnableCursor(); 
}

void Renderer::stopRender() {
    isRendering = false;
    isPreview = true; 
}

void Renderer::update(float dt) {
    if (isRendering) return;

    if (IsKeyPressed(KEY_TAB)) {
        if (IsCursorHidden()) EnableCursor();
        else DisableCursor();
    }

    bool moved = false;

    if (IsCursorHidden()) {
        Vector2 delta = GetMouseDelta();
        if (delta.x != 0 || delta.y != 0) {
            camParams.yaw   += delta.x * settings.mouseSens;
            camParams.pitch -= delta.y * settings.mouseSens;
            
            if (camParams.pitch > 89.0f) camParams.pitch = 89.0f;
            if (camParams.pitch < -89.0f) camParams.pitch = -89.0f;
            
            moved = true;
        }

        // Отримуємо вектори напрямку з ПОТОЧНИХ кутів, а не зі старого lookAt
        // Це виправить проблему "інвертованого руху"
        float yawRad   = camParams.yaw   * (3.14159265f / 180.0f);
        float pitchRad = camParams.pitch * (3.14159265f / 180.0f);
        
        Vec3 forward;
        forward.x = std::cos(yawRad) * std::cos(pitchRad);
        forward.y = std::sin(pitchRad);
        forward.z = std::sin(yawRad) * std::cos(pitchRad);
        forward = normalize(forward);
        
        Vec3 right = normalize(cross(forward, camParams.vUp));
        
        Vec3 velocity = {0,0,0};
        // Тепер W завжди летить туди, куди дивиться камера
        if (IsKeyDown(KEY_W)) velocity += forward;
        if (IsKeyDown(KEY_S)) velocity -= forward;
        if (IsKeyDown(KEY_D)) velocity += right;
        if (IsKeyDown(KEY_A)) velocity -= right;
        if (IsKeyDown(KEY_Q)) velocity -= Vec3(0,1,0);
        if (IsKeyDown(KEY_E)) velocity += Vec3(0,1,0); 

        if (velocity.lengthSq() > 0) {
            camParams.lookFrom += normalize(velocity) * settings.moveSpeed * dt;
            moved = true;
        }
    }

    if (moved) {
        camera.updateViewMatrix(camParams); 
        reset();       
    }
}

// Функція для чистого прев'ю (без шуму)
Vec3 Renderer::tracePreview(const Ray& r) {
    HitRecord rec;
    if (scene->hit(r, 0.001f, 1e9f, rec)) {
        // Повертаємо чистий колір матеріалу.
        // Це виглядає як векторна графіка, зовсім без шуму.
        return rec.mat.albedo; 
    }
    return Vec3(0.15f, 0.15f, 0.15f); // Чистий сірий фон
}

// Функція для фотореалізму (з шумом, який зникає з часом)
Vec3 Renderer::traceRay(const Ray& r, int depth) {
    if (depth <= 0) return {0, 0, 0};
    HitRecord rec;
    if (scene->hit(r, 0.001f, 1e9f, rec)) {
        Ray  scattered;
        Vec3 attenuation;
        if (rec.mat.scatter(r, rec, attenuation, scattered))
            return attenuation * traceRay(scattered, depth - 1);
        return {0, 0, 0};
    }
    Vec3  unit = normalize(r.direction);
    float t    = 0.5f * (unit.y + 1.0f);
    return (1.0f - t) * Vec3{1, 1, 1} + t * Vec3{0.5f, 0.7f, 1.0f};
}

void Renderer::renderSample() {
    int W = settings.width, H = settings.height;

    // 1. РЕЖИМ ПОЛЬОТУ (Preview)
    // Тут ми малюємо ідеально чисту картинку (без шуму)
    if (!isRendering) {
        samplesDone = 1; 
        #pragma omp parallel for schedule(dynamic)
        for (int j = 0; j < H; ++j) {
            for (int i = 0; i < W; ++i) {
                float u = (i + 0.5f) / (W - 1);
                float v = (j + 0.5f) / (H - 1);
                Ray r = camera.getRay(u, v);
                accumBuffer[j * W + i] = tracePreview(r);
            }
        }
        uploadPixels();
        return;
    }

    // 2. РЕЖИМ РЕНДЕРУ (Start Render)
    // Тут шум БУДЕ спочатку, це нормально. Треба просто почекати.
    if (samplesDone >= settings.samplesTarget) return; // Виправлено доступ

    #pragma omp parallel for schedule(dynamic)
    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            float u = (i + randomFloat()) / (W - 1);
            float v = (j + randomFloat()) / (H - 1);
            Ray r = camera.getRay(u, v);
            Vec3 pixelColor = traceRay(r, settings.maxBounces);
            
            int idx = j * W + i;
            if (samplesDone == 0) accumBuffer[idx] = pixelColor;
            else accumBuffer[idx] += pixelColor;
        }
    }
    samplesDone++;
    uploadPixels();
}

void Renderer::uploadPixels() {
    int W = settings.width, H = settings.height;
    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            int srcIdx = j * W + i;
            Color c = accumBuffer[srcIdx].toColor(samplesDone > 0 ? samplesDone : 1);
            
            int destIdx = (j * W + i) * 4;
            pixels[destIdx+0] = c.r;
            pixels[destIdx+1] = c.g;
            pixels[destIdx+2] = c.b;
            pixels[destIdx+3] = 255;
        }
    }
    UpdateTexture(outputTex, pixels.data());
}

}