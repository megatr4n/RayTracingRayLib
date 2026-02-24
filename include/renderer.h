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

enum class ObjType { None, Sphere, Quad, Mesh };

    struct Selection {
        ObjType type = ObjType::None;
        int index = -1;
    };

class Renderer {
public:
    tf::Executor executor; 

    RendererSettings settings;
    RtCameraParams   camParams;
    Scene* scene       = nullptr;
    RtCamera         camera;
    Selection selection;

    int draggingAxis = -1;  
    float initialDragT = 0.0f;
    Vec3 initialObjPos;
    
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

    void drawRaylibPreview();

    void startRender();
    void stopRender();
    void saveRenderToPNG(const std::string& filename);

private:
    void uploadPixels();
    Vec3 traceRay(const Ray& r, int depth);
    Vec3 tracePreview(const Ray& r);
};
}