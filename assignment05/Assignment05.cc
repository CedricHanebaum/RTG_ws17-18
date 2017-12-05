#include "Assignment05.hh"

#include <cstdint>
#include <vector>

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/camera/GenericCamera.hh>

// AntTweakBar
#include <AntTweakBar.h>

// GLFW
#include <GLFW/glfw3.h>

// extra helper
#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/UVSphere.hh>
#include <glow-extras/timing/PerformanceTimer.hh>

// in the implementation, we want to omit the glow:: prefix
using namespace glow;


void Assignment05::update(float elapsedSeconds)
{
    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    // update rigid body
    {
        // accumulate forces
        mRigidBody.clearForces();
        if (length(mThrusterDir) > 0.0f)
        {
            auto thrusterWorldPos = mRigidBody.pointLocalToGlobal(mThrusterPos);
            auto thrusterWorldDir = mRigidBody.directionLocalToGlobal(mThrusterDir);
            auto force = -thrusterWorldDir * mThrusterStength;
            mRigidBody.addForce(force, thrusterWorldPos);
        }

        // update RB
        mRigidBody.update(elapsedSeconds);

        // perform collision checks
        mRigidBody.checkPlaneCollision({0, 0, 0}, {0, 1, 0}); // against ground plane

        // fix center of mass
        if (mFixCenterOfMass)
        {
            mRigidBody.linearMomentum = {0, 0, 0};
            mRigidBody.linearPosition = {0, 3, 0};
        }
    }
}

void Assignment05::render(float elapsedSeconds)
{
    // Functions called with this macro are undone at the end of the scope
    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);

    // draw shadow map
    {
        auto fb = mFramebufferShadow->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        // set up shadow camera
        mShadowCamera.setPosition(mLightPos);
        mShadowCamera.setViewMatrix(glm::lookAt(mLightPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
        mShadowCamera.setProjectionMatrix(glm::perspective(glm::radians(90.0f), 1.0f, 10.0f, 50.0f));
        mShadowCamera.setViewportSize({mShadowMapSize, mShadowMapSize});

        // render scene from light
        renderScene(&mShadowCamera, true);
    }

    // draw scene
    {
        // clear the screen
        glClearColor(0.3f, 0.3f, 0.3f, 1.00f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // render scene normally
        renderScene(getCamera().get(), false);
    }
}

void Assignment05::renderScene(camera::CameraBase* cam, bool shadowPass)
{
    // Get camera matrices
    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 proj = cam->getProjectionMatrix();

    // configure line shader
    {
        auto shader = mShaderLine->use();
        shader.setUniform("uViewMatrix", view);
        shader.setUniform("uProjectionMatrix", proj);
    }

    // draw objects
    {
        auto shader = mShaderObj->use();
        shader.setUniform("uViewMatrix", view);
        shader.setUniform("uProjectionMatrix", proj);

        // set up lighting
        if (!shadowPass)
        {
            shader.setUniform("uLightPos", mLightPos);
            shader.setUniform("uCamPos", cam->getPosition());

            shader.setTexture("uShadowMap", mTextureShadow);
            shader.setUniform("uShadowViewProj", mShadowCamera.getProjectionMatrix() * mShadowCamera.getViewMatrix());
        }

        // draw plane
        {
            shader.setUniform("uModelMatrix", glm::mat4());
            shader.setUniform("uColor", glm::vec3(0.00f, 0.33f, 0.66f));
            mMeshPlane->bind().draw();
        }

        // draw rigid body
        {
            // solid color
            shader.setUniform("uColor", glm::vec3(0.66f, 0.00f, 0.33f));

/// Task 1.b
/// Draw the rigid body
///
/// Your job is to:
///     - compute the model matrix of the box or sphere
///     - pass it to the shader as uniform
///
/// Notes:
///     - a rigid body can consist of several shapes with one common
///       transformation matrix (rbTransform)
///     - furthermore, every shape is transformed by its own matrix (s->transform)
///       and some scaling that is defined by the radius (sphere) or halfExtent (box)
///
/// ============= STUDENT CODE BEGIN =============

            for (auto const& s : mRigidBody.shapes)
            {
                auto sphere = std::dynamic_pointer_cast<SphereShape>(s);
                auto box = std::dynamic_pointer_cast<BoxShape>(s);

                auto rbTransform = mRigidBody.getTransform();

                // draw spheres
                if (sphere)
                {
                    glm::mat4 modelMatrix = glm::mat4();
                    auto r = sphere->radius;
                    modelMatrix *= rbTransform;
                    modelMatrix *= sphere->transform;
                    modelMatrix = glm::scale(modelMatrix, glm::vec3(r, r, r));

                    shader.setUniform("uModelMatrix", modelMatrix);

                    mMeshSphere->bind().draw();
                }

                // draw boxes
                if (box)
                {
                    glm::mat4 modelMatrix = glm::mat4();
                    modelMatrix *= rbTransform;
                    modelMatrix *= box->transform;
                    modelMatrix = glm::scale(modelMatrix, box->halfExtent);

                    shader.setUniform("uModelMatrix", modelMatrix);

                    mMeshCube->bind().draw();
                }
            }

/// ============= STUDENT CODE END =============
        }
    }

    // draw lines
    {
        switch (mRigidBody.preset)
        {
        case Preset::Pendulum:
            // white line from rigid body to pendulum
            drawLine(mRigidBody.linearPosition, mRigidBody.pendulumPosition, {1, 1, 1});
            break;

        default: // nothing
            break;
        }

        // debug rendering
        if (!shadowPass)
        {
            GLOW_SCOPED(disable, GL_DEPTH_TEST); // draw above everything

            if (mShowObjectFrame)
            {
                auto pc = mRigidBody.pointLocalToGlobal({0, 0, 0});
                auto px = mRigidBody.pointLocalToGlobal({1, 0, 0});
                auto py = mRigidBody.pointLocalToGlobal({0, 1, 0});
                auto pz = mRigidBody.pointLocalToGlobal({0, 0, 1});
                drawLine(pc, px, {1, 0, 0});
                drawLine(pc, py, {0, 1, 0});
                drawLine(pc, pz, {0, 0, 1});
            }

            if (mShowRayHit)
            {
                glm::vec3 pos, dir;
                getMouseRay(pos, dir);

                glm::vec3 hit, normal;
                if (mRigidBody.rayCast(pos, dir, hit, normal))
                    drawLine(hit, hit + normal, {1, 0, 1});
            }

            // draw thruster
            if (length(mThrusterDir) > 0.0f)
            {
                auto pos = mRigidBody.pointLocalToGlobal(mThrusterPos);
                auto dir = mRigidBody.directionLocalToGlobal(mThrusterDir);
                drawLine(pos, pos + dir, {1, 1, 0});
            }
        }
    }
}

bool Assignment05::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    if (action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        // calculate mouse ray
        glm::vec3 pos, dir;
        getMouseRay(pos, dir);

        glm::vec3 hit, normal;
        if (mRigidBody.rayCast(pos, dir, hit, normal))
        {
            // impact or thruster
            if (mods & GLFW_MOD_SHIFT) // shift + [LMB] -> thruster
            {
                mThrusterDir = mRigidBody.directionGlobalToLocal(normal);
                mThrusterPos = mRigidBody.pointGlobalToLocal(hit);
            }
            else // [LMB] -> impact
            {
                auto impulse = mImpulseStength * dir;
                mRigidBody.applyImpulse(impulse, hit);
            }
        }
        else
        {
            // clear thruster
            if (mods & GLFW_MOD_SHIFT)
                mThrusterDir = glm::vec3();
        }
    }

    return false;
}

void Assignment05::loadPreset(Preset preset)
{
    mThrusterDir = {0, 0, 0};
    mRigidBody.loadPreset(preset);
    mRigidBody.calculateMassAndInertia();
}


/// Task 1.a
/// Build the line mesh
/// Draw the line mesh
///
/// Your job is to:
///     - build a line mesh for one single line
///
/// Notes:
///     - store the VertexArray to the member variable mMeshLine
///     - the primitive type is GL_LINES
///     - for the drawing, you have to set some uniforms for mShaderLine
///     - uViewMatrix and uProjectionMatrix are automatically set
///
///
/// ============= STUDENT CODE BEGIN =============

void Assignment05::buildLineMesh()
{
    // TODO
}

void Assignment05::drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color)
{
    // TODO
}

/// ============= STUDENT CODE END =============

void Assignment05::getMouseRay(glm::vec3& pos, glm::vec3& dir)
{
    auto mp = getMousePosition();
    auto x = mp.x;
    auto y = mp.y;

    auto cam = getCamera();
    glm::vec3 ps[2];
    auto i = 0;
    for (auto d : {0.5f, -0.5f})
    {
        glm::vec4 v{x / float(getWindowWidth()) * 2 - 1, 1 - y / float(getWindowHeight()) * 2, d * 2 - 1, 1.0};

        v = glm::inverse(cam->getProjectionMatrix()) * v;
        v /= v.w;
        v = glm::inverse(cam->getViewMatrix()) * v;
        ps[i++] = glm::vec3(v);
    }

    pos = cam->getPosition();
    dir = normalize(ps[0] - ps[1]);
}

static void TW_CALL PresetPendulumBtn(void* data)
{
    ((Assignment05*)data)->loadPreset(Preset::Pendulum);
}
static void TW_CALL PresetRolyPolyToyBtn(void* data)
{
    ((Assignment05*)data)->loadPreset(Preset::RolyPolyToy);
}

void Assignment05::init()
{
    // limit GPU to 60 fps
    setVSync(true);
    // we don't use the GlfwApp built-in rendering
    setUseDefaultRendering(false);

    GlfwApp::init(); // Call to base GlfwApp

    // set up camera
    auto cam = getCamera();
    cam->setPosition({12, 12, 12});
    cam->setTarget({0, 0, 0});

    // load shaders
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shader/obj");
    mShaderLine = Program::createFromFile(util::pathOf(__FILE__) + "/shader/line");

    // shadow map
    mTextureShadow = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_DEPTH_COMPONENT32F, 1);
    mTextureShadow->bind().setMinFilter(GL_LINEAR);
    mFramebufferShadow = Framebuffer::create({}, mTextureShadow); // depth-only

    // create geometry
    mMeshSphere = geometry::UVSphere<>().generate();
    mMeshCube = geometry::Cube<>().generate();
    mMeshPlane = geometry::Quad<geometry::CubeVertex>().generate([](float u, float v) {
        return geometry::CubeVertex{glm::vec3((u * 2 - 1) * 10000, 0, (v * 2 - 1) * 10000), //
                                    glm::vec3(0, 1, 0),                                     //
                                    glm::vec3(1, 0, 0),                                     //
                                    glm::vec2(u, v)};
    });
    buildLineMesh();

    // setup tweakbar
    {
        TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &mLightPos, "group=scene");
        TwAddVarRW(tweakbar(), "Gravity", TW_TYPE_FLOAT, &mRigidBody.gravity, "group=scene step=0.01");
        TwAddVarRW(tweakbar(), "Impulse Strength", TW_TYPE_FLOAT, &mImpulseStength, "group=scene step=0.01");
        TwAddVarRW(tweakbar(), "Thruster Strength", TW_TYPE_FLOAT, &mThrusterStength, "group=scene step=0.01");
        TwAddVarRW(tweakbar(), "Center of Mass", TW_TYPE_BOOLCPP, &mFixCenterOfMass, "group=scene");
        TwAddButton(tweakbar(), "Preset: Pendulum", PresetPendulumBtn, this, "group=scene");
        TwAddButton(tweakbar(), "Preset: Roly-poly toy", PresetRolyPolyToyBtn, this, "group=scene");

        TwAddVarRW(tweakbar(), "Object Frame", TW_TYPE_BOOLCPP, &mShowObjectFrame, "group=rendering");
        TwAddVarRW(tweakbar(), "Ray Hit", TW_TYPE_BOOLCPP, &mShowRayHit, "group=rendering");

        TwDefine("Tweakbar size='220 220' valueswidth=60");
    }

    // init scene
    loadPreset(Preset::Pendulum);
    // loadPreset(Preset::RolyPolyToy);
}
