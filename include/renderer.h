#pragma once
#include "camera.h"
#include "hittable.h"
#include <vector>
#include <raylib.h>

struct RendererSettings {
    int width         = 800;
    int height        = 450;
    int maxBounces    = 8;
    int samplesTarget = 256;
};

class Renderer {
public:
    RendererSettings settings;
    CameraParams     camParams;
    Scene*           scene       = nullptr;
    Camera           camera;
    Texture2D        outputTex   = {};
    int              samplesDone = 0;

    std::vector<Vec3>          accumBuffer;
    std::vector<unsigned char> pixels;

    void init(Scene* s);
    void resize(int w, int h);
    void reset();
    void renderSample();

private:
    void uploadPixels();
    Vec3 traceRay(const Ray& r, int depth);
};