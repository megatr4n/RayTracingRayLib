#pragma once
#include "camera.h"
#include "hittable.h"
#include <vector>
#include <raylib.h>

#include <taskflow/taskflow.hpp>

#include <taskflow/algorithm/for_each.hpp>

namespace rt {

struct RendererSettings {
    int width         = 800;
    int height        = 450;
    int maxBounces    = 8;
    int samplesTarget = 256;
    float moveSpeed   = 5.0f; 
    float mouseSens   = 0.2f; 
};

class Renderer {
public:
    tf::Executor executor; 

    RendererSettings settings;
    RtCameraParams   camParams;
    Scene* scene       = nullptr;
    RtCamera         camera;
    Texture2D        outputTex   = {};
    int samplesDone = 0;
    
    bool isPreview   = true;
    bool isRendering = false;

    std::vector<Vec3>          accumBuffer;
    std::vector<unsigned char> pixels;

    void init(Scene* s);
    void update(float dt);
    void resize(int w, int h);
    void reset();
    void renderSample();

    void startRender();
    void stopRender();

private:
    void uploadPixels();
    Vec3 traceRay(const Ray& r, int depth);
    Vec3 tracePreview(const Ray& r);
};
}