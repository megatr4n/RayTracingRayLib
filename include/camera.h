#pragma once
#include "ray.h"

namespace rt {
struct RtCameraParams {
    Vec3  lookFrom    = {13, 2, 3};
    Vec3  lookAt      = {0, 0, 0};
    Vec3  vUp         = {0, 1, 0};
    float vfov        = 20.0f;
    float aspectRatio = 16.0f / 9.0f;
    float aperture    = 0.1f;
    float focusDist   = 10.0f;

    float yaw         = 0.0f; 
    float pitch       = 0.0f;
};

class RtCamera {
public:
    Vec3  origin, lowerLeftCorner, horizontal, vertical, u, v, w;
    float lensRadius = 0;

    void init(const RtCameraParams& p);
    Ray  getRay(float s, float t) const;

    void updateViewMatrix(RtCameraParams& p);
};
}