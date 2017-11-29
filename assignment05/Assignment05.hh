#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/glfw/GlfwApp.hh>

#include "RigidBody.hh"
#include "Shapes.hh"

/**
 * Assignment05: A rigid body simulation without collisions
 */
class Assignment05 : public glow::glfw::GlfwApp
{
private: // input and camera
    glm::vec3 mLightPos = {-10, 30, 5};

private: // scene
    RigidBody mRigidBody;

    float mImpulseStength = 10.0f;
    float mThrusterStength = 10.0f;

    // thruster is in rigidbody-local space
    glm::vec3 mThrusterPos;
    glm::vec3 mThrusterDir;

    // if true, we fix the center of mass
    bool mFixCenterOfMass = false;

private: // graphics
    glow::SharedVertexArray mMeshSphere;
    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mMeshPlane;
    glow::SharedVertexArray mMeshLine;

    glow::SharedProgram mShaderObj;
    glow::SharedProgram mShaderLine;

    const int mShadowMapSize = 2048;
    glow::SharedTexture2D mTextureShadow;
    glow::SharedFramebuffer mFramebufferShadow;
    glow::camera::FixedCamera mShadowCamera;

    bool mShowObjectFrame = !false;
    bool mShowRayHit = !false;

private: // helper
    void buildLineMesh();
    void drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color);

    void getMouseRay(glm::vec3& pos, glm::vec3& dir);

public:
    void loadPreset(Preset preset);

    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;

    void renderScene(glow::camera::CameraBase* cam, bool shadowPass);

    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
};
