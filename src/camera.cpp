#include "camera.h"
#include "random_utils.h"
#include <cmath>

namespace rt {

void RtCamera::init(const RtCameraParams& p) {
    lensRadius = p.aperture / 2.0f;
    float theta = p.vfov * (3.14159265f / 180.0f);
    float h     = std::tan(theta / 2.0f);
    float vpH   = 2.0f * h;
    float vpW   = p.aspectRatio * vpH;

    w = normalize(p.lookFrom - p.lookAt);
    u = normalize(cross(p.vUp, w));
    v = cross(w, u);

    origin          = p.lookFrom;
    horizontal      = p.focusDist * vpW * u;
    vertical        = p.focusDist * vpH * v;
    lowerLeftCorner = origin - horizontal/2.0f - vertical/2.0f - p.focusDist * w;
}

Ray RtCamera::getRay(float s, float t) const {
    Vec3 rd     = lensRadius * randomInUnitDisk();
    Vec3 offset = u * rd.x + v * rd.y;
    Vec3 dir    = lowerLeftCorner + s*horizontal + t*vertical - origin - offset;
    return Ray(origin + offset, normalize(dir));
}

void RtCamera::updateViewMatrix(RtCameraParams& p) {
    float yawRad = p.yaw * (3.14159265f / 180.0f);
    float pitchRad = p.pitch * (3.14159265f / 180.0f);

    Vec3 front;
    front.x = std::cos(yawRad) * std::cos(pitchRad);
    front.y = std::sin(pitchRad);
    front.z = std::sin(yawRad) * std::cos(pitchRad);
    p.lookAt = p.lookFrom + normalize(front);
    init(p);
}
}