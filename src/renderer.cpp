#include "renderer.h"
#include "random_utils.h"
#include <algorithm>
#include <cmath>

// ------------------------------------------------------------------ init / resize

void Renderer::init(Scene* s) {
    scene = s;
    resize(settings.width, settings.height);
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
    camera.init(camParams);
}

// ------------------------------------------------------------------ render loop

void Renderer::renderSample() {
    if (samplesDone >= settings.samplesTarget) return;

    int W = settings.width;
    int H = settings.height;

    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            float u = (i + randomFloat()) / (W - 1);
            float v = (j + randomFloat()) / (H - 1);
            accumBuffer[j * W + i] += traceRay(camera.getRay(u, v), settings.maxBounces);
        }
    }

    samplesDone++;
    uploadPixels();
}

// ------------------------------------------------------------------ private

void Renderer::uploadPixels() {
    int W = settings.width;
    int H = settings.height;

    for (int j = 0; j < H; ++j) {
        for (int i = 0; i < W; ++i) {
            Color c   = accumBuffer[j * W + i].toColor(samplesDone);
            int   idx = ((H - 1 - j) * W + i) * 4; // flip Y for OpenGL convention
            pixels[idx + 0] = c.r;
            pixels[idx + 1] = c.g;
            pixels[idx + 2] = c.b;
            pixels[idx + 3] = 255;
        }
    }

    UpdateTexture(outputTex, pixels.data());
}

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

    // Sky gradient
    Vec3  unit = normalize(r.direction);
    float t    = 0.5f * (unit.y + 1.0f);
    return (1.0f - t) * Vec3{1, 1, 1} + t * Vec3{0.5f, 0.7f, 1.0f};
}