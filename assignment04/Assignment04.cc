#include "Assignment04.hh"

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/VertexArray.hh>

#include <glow-extras/camera/GenericCamera.hh>

// AntTweakBar
#include <AntTweakBar.h>

// GLFW
#include <GLFW/glfw3.h>

// extra helper
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/UVSphere.hh>
#include <glow-extras/timing/PerformanceTimer.hh>

#include <cstdint>

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

void Assignment04::update(float elapsedSeconds)
{
    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    mAccumSeconds += elapsedSeconds;

    // update sphere
    if (mSphereMoving)
    {
        mSpherePos = glm::vec3(mSphereMovingRadius * glm::cos(mAccumSeconds), //
                               -1 - mSphereRadius,                            //
                               mSphereMovingRadius * glm::sin(mAccumSeconds));
    }
    else
    {
        mSpherePos = {0, -1 - mSphereRadius, 0};
    }

    static auto fixedParticles = mCloth.fixedParticles;
    if (mCloth.fixedParticles != fixedParticles)
    {
        fixedParticles = mCloth.fixedParticles;
        mCloth.resetMotionState();
    }

    // update cloth
    mCloth.updateInit(elapsedSeconds);
    mCloth.updateForces();
    mCloth.updateMotion(elapsedSeconds);
    mCloth.addSphereCollision(mSpherePos, mSphereRadius);
}

void Assignment04::render(float elapsedSeconds)
{
    // Functions called with this macro are undone at the end of the scope
    GLOW_SCOPED(enable, GL_DEPTH_TEST);

    // clear the screen
    glClearColor(0.30f, 0.3f, 0.3f, 1.00f);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Get camera matrices
    glm::mat4 view = getCamera()->getViewMatrix();
    glm::mat4 proj = getCamera()->getProjectionMatrix();

    // set up shader
    auto shader = mShaderObj->use();
    shader.setUniform("uViewMatrix", view);
    shader.setUniform("uProjectionMatrix", proj);
    shader.setUniform("uLightPos", mLightPos);
    shader.setUniform("uCamPos", getCamera()->getPosition());
    shader.setUniform("uShowNormals", mShowNormals);

    // draw sphere
    {
        auto modelMatrix = glm::translate(mSpherePos) * glm::scale(glm::vec3(mSphereRadius - mSphereOffset));
        shader.setUniform("uModelMatrix", modelMatrix);
        shader.setUniform("uTranslucency", 0.0f);

        mSphere->bind().draw();
    }

    // draw cloth
    {
        glm::mat4 modelMatrix = glm::mat4();
        shader.setUniform("uModelMatrix", modelMatrix);
        shader.setUniform("uTranslucency", 0.3f);

        mCloth.createVAO()->bind().draw();
    }
}

bool Assignment04::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    return onMousePosition(x, y);
}

bool Assignment04::onMousePosition(double x, double y)
{
    if (TwEventMousePosGLFW(window(), x, y))
        return true;

    if (isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        isMousePressed = true;

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

        auto pos = cam->getPosition();
        auto dir = normalize(ps[0] - ps[1]);

        mCloth.drag(pos, dir, cam->getForwardDirection());

        return true;
    }
    else if (isMousePressed)
    {
        isMousePressed = false;
        mCloth.releaseDrag();
        return true;
    }

    return GlfwApp::onMousePosition(x, y);
}

static void TW_CALL ResetClothButton(void *clientData)
{
    ((Cloth *)clientData)->resetMotionState();
}

void Assignment04::init()
{
    GlfwApp::init(); // Call to base GlfwApp

    // set up camera
    auto cam = getCamera();
    cam->setPosition({2, 2, 2});
    cam->setTarget({0, 0, 0});

    // load resources
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shaderObj");
    mSphere = geometry::UVSphere<Vertex>().generate([](glm::vec3 position, glm::vec3 normal, glm::vec3 tangent, glm::vec2 texCoord) {
        glm::vec3 sphereCol0 = {0.66, 0.00, 0.33};
        glm::vec3 sphereCol1 = sphereCol0;

        return Vertex{position, //
                      normal,   //
                      (int(glm::mod(6.0f * texCoord.x, 1.0f) > 0.5f) % 2) ? sphereCol0 : sphereCol1};
    });

    // create cloth object
    mCloth = Cloth(30, 10.0f, 10);

    // setup tweakbar
    {
        TwAddVarRW(tweakbar(), "Sphere Radius", TW_TYPE_FLOAT, &mSphereRadius, "group=scene step=0.1");
        TwAddVarRW(tweakbar(), "Moving Sphere", TW_TYPE_BOOLCPP, &mSphereMoving, "group=scene");
        TwAddVarRW(tweakbar(), "Moving Radius", TW_TYPE_FLOAT, &mSphereMovingRadius, "group=scene step=0.1");
        TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &mLightPos, "group=scene");
        TwAddButton(tweakbar(), "Reset Cloth", ResetClothButton, &mCloth, "group=scene");

        TwAddVarRW(tweakbar(), "Sphere Offset", TW_TYPE_FLOAT, &mSphereOffset, "group=rendering step=0.01");
        TwAddVarRW(tweakbar(), "Smooth Shading", TW_TYPE_BOOLCPP, &mCloth.smoothShading, "group=rendering");
        TwAddVarRW(tweakbar(), "Show Normals", TW_TYPE_BOOLCPP, &mShowNormals, "group=rendering");

        TwAddVarRW(tweakbar(), "k", TW_TYPE_FLOAT, &mCloth.springK, "group=constants step=0.1");
        TwAddVarRW(tweakbar(), "d", TW_TYPE_FLOAT, &mCloth.dampingD, "group=constants step=0.01");
        TwAddVarRW(tweakbar(), "g", TW_TYPE_FLOAT, &mCloth.gravity, "group=constants step=0.1");

        TwEnumVal coloringEV[] = {
            {(int)Coloring::Smooth, "Smooth"},   //
            {(int)Coloring::Stress, "Stress"},   //
            {(int)Coloring::Checker, "Checker"}, //
        };
        TwType coloringType = TwDefineEnum("Coloring", coloringEV, 3);
        TwAddVarRW(tweakbar(), "Coloring", coloringType, &mCloth.coloring, "group=rendering");

        TwEnumVal fixedEV[] = {
            {(int)FixedParticles::OneSide, "One Side"},     //
            {(int)FixedParticles::TwoSides, "Two Sides"},   //
            {(int)FixedParticles::FourSides, "Four Sides"}, //
            {(int)FixedParticles::Corners, "Corners"},      //
        };
        TwType fixedType = TwDefineEnum("Fixed", fixedEV, 4);
        TwAddVarRW(tweakbar(), "Fixed Particles", fixedType, &mCloth.fixedParticles, "group=scene");
    }
}
