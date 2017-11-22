#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include "Cloth.hh"
/**
 * Assignment04: A cloth simulation assignment to cover the deformable bodies topic (in two dimensions)
 */
class Assignment04 : public glow::glfw::GlfwApp
{
private: // input and camera
    glm::vec3 mLightPos = {-1, 10, 5};

    bool isMousePressed = false;

private: // scene
    float mAccumSeconds = 0;

    Cloth mCloth;

    glm::vec3 mSpherePos;
    float mSphereRadius = 2.0f;
    bool mSphereMoving = false;
    float mSphereMovingRadius = 2.5f;
    float mSphereOffset = 0.09f;

private: // graphics
    glow::SharedVertexArray mPlane;
    glow::SharedVertexArray mSphere;

    glow::SharedProgram mShaderObj;

    bool mShowNormals = false;

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;

    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onMousePosition(double x, double y) override;
};
