#pragma once

#include <array>

#include <glm/glm.hpp>

#include <glow-extras/camera/CameraBase.hh>

struct FrustumCuller
{
private:
    std::array<glm::vec4, 6> planes;
    glm::vec3 camPos;
    bool isShadow;

    glm::vec3 unproject(glm::vec3 sp, glm::mat4 const& invView, glm::mat4 const& invProj) const
    {
        auto vp = invProj * glm::vec4(sp, 1.0);
        vp /= vp.w;
        return glm::vec3(invView * vp);
    }

    glm::vec4 plane(glm::vec3 p0, glm::vec3 p1, glm::vec3 p2) const
    {
        auto n = normalize(cross(p1 - p0, p2 - p0));
        return glm::vec4(n, dot(n, p0));
    }

public:
    FrustumCuller(glow::camera::CameraBase const& cam, bool isShadow) : isShadow(isShadow)
    {
        camPos = cam.getPosition();

        auto invView = inverse(cam.getViewMatrix());
        auto invProj = inverse(cam.getProjectionMatrix());

        auto p000 = unproject({-1.0f, -1.0f, -1.0f}, invView, invProj);
        auto p001 = unproject({-1.0f, -1.0f, +1.0f}, invView, invProj);
        auto p010 = unproject({-1.0f, +1.0f, -1.0f}, invView, invProj);
        auto p011 = unproject({-1.0f, +1.0f, +1.0f}, invView, invProj);
        auto p100 = unproject({+1.0f, -1.0f, -1.0f}, invView, invProj);
        auto p101 = unproject({+1.0f, -1.0f, +1.0f}, invView, invProj);
        auto p110 = unproject({+1.0f, +1.0f, -1.0f}, invView, invProj);
        // auto p111 = unproject({+1.0f, +1.0f, +1.0f}, invView, invProj);

        // near
        planes[0] = plane(p000, p100, p010);
        // far
        planes[1] = plane(p001, p011, p101);

        // left
        planes[2] = plane(p000, p010, p001);
        // right
        planes[3] = plane(p100, p101, p110);

        // bottom
        planes[4] = plane(p000, p001, p100);
        // top
        planes[5] = plane(p010, p110, p011);
    }

    bool isAabbVisible(glm::vec3 amin, glm::vec3 amax) const
    {
        auto center = (amin + amax) / 2.0f;
        auto radius = distance(amax, center);

        for (auto i = 0; i < 6; ++i)
        {
            auto const& p = planes[i];
            auto dis = dot(center, glm::vec3(p));
            if (dis - radius > p.w)
                return false;
        }

        return true;
    }

    bool isAabbInRange(glm::vec3 amin, glm::vec3 amax, float renderDistance) const
    {
        auto p = clamp(camPos, amin, amax);

        if (distance(p, camPos) > renderDistance)
            return false;

        return true;
    }

    bool isFaceVisible(glm::ivec3 dir, glm::vec3 amin, glm::vec3 amax) const
    {
        auto n = glm::vec3(dir);

        if (isShadow)
            return dot(camPos, n) > 0; // camPos == lightDir

        auto aabbDis = glm::min(dot(n, amin), dot(n, amax));
        auto camDis = dot(camPos, n);

        return camDis > aabbDis;
    }
};
