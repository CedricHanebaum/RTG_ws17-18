#include "Assignment07.hh"

// System headers
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
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/TextureCubeMap.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow/data/TextureData.hh>

#include <glow-extras/assimp/Importer.hh>
#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/geometry/Quad.hh>


// AntTweakBar
#include <AntTweakBar.h>

// GLFW
#include <GLFW/glfw3.h>

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

void Assignment07::update(float elapsedSeconds)
{
    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    mRuntime += elapsedSeconds;

    // TODO: game logic is coming later
}

void Assignment07::render(float elapsedSeconds)
{
    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);
    if (!mBackFaceCulling)
        glDisable(GL_CULL_FACE);

    // renormalize light dir (tweakbar might have changed it)
    mLightDir = normalize(mLightDir);

    // draw shadow map
    {
        updateShadowMapTexture();
        auto fb = mFramebufferShadow->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        // set up shadow camera
        auto shadowCamDis = 150.0f;
        auto shadowSize = 150.0f; // FIXME: dynamic bias: mRenderDistance * 1.5f + mWorld.chunkSize * 2;
        auto shadowPos = mLightDir * shadowCamDis;
        mShadowCamera.setPosition(shadowPos);
        mShadowCamera.setViewMatrix(lookAt(shadowPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
        mShadowCamera.setProjectionMatrix(glm::ortho(-shadowSize, shadowSize, -shadowSize, shadowSize, 1.0f, shadowCamDis * 2.0f));
        mShadowCamera.setViewportSize({mShadowMapSize, mShadowMapSize});

        // render scene from light
        if (mEnableShadows)
            renderScene(&mShadowCamera, RenderPass::Shadow);
    }

    // draw opaque scene
    {
        auto fb = mFramebufferOpaque->bind();

        // clear the screen
        GLOW_SCOPED(clearColor, 0.3f, 0.3f, 0.3f, 1.00f);
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

        // debug: wireframe rendering
        GLOW_SCOPED(polygonMode, GL_FRONT_AND_BACK, mWireframe ? GL_LINE : GL_FILL);

        // render scene normally
        renderScene(getCamera().get(), RenderPass::Opaque);
    }

    // draw transparent scene
    {
        auto fb = mFramebufferFinal->bind();

        // clear depth
        glClear(GL_DEPTH_BUFFER_BIT);

        // copy opaque color
        {
            GLOW_SCOPED(disable, GL_DEPTH_TEST);
            GLOW_SCOPED(disable, GL_CULL_FACE);

            auto shader = mShaderCopy->use();
            shader.setTexture("uTexture", mTexOpaqueColor);

            mMeshQuad->bind().draw();
        }

        // debug: wireframe rendering
        GLOW_SCOPED(polygonMode, GL_FRONT_AND_BACK, mWireframe ? GL_LINE : GL_FILL);

        // render translucent part of scene (e.g. water)
        renderScene(getCamera().get(), RenderPass::Transparent);
    }

    // draw output
    {
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        auto shader = mShaderOutput->use();
        shader.setTexture("uTexture", mTexFinalColor);
        shader.setUniform("uUseFXAA", mUseFXAA);
        shader.setUniform("uUseDithering", mUseDithering);

        mMeshQuad->bind().draw();
    }
}

void Assignment07::rebuildWorld()
{
    mWorld.clearChunks();

    // ensure proper chunks exist
    auto refPos = (glm::ivec3)round(mPlayerPos);

    auto rad = (int)mRenderDistance;
    auto radY = 32;
    for (auto z = -rad; z <= rad; ++z)
        for (auto y = -radY; y <= radY; ++y)
            for (auto x = -rad; x <= rad; ++x)
                mWorld.ensureChunkAt(refPos + glm::ivec3(x, y, z));
}

void Assignment07::renderScene(camera::CameraBase* cam, RenderPass pass)
{
    // set up general purpose shaders
    switch (pass)
    {
    case RenderPass::Opaque:
        setUpShader(mShaderLine, cam, pass);
        break;
    case RenderPass::Transparent:
        setUpShader(mShaderOverlay, cam, pass);
        break;
    default:
        break;
    }

    // draw background
    if (pass == RenderPass::Opaque)
    {
        setUpShader(mShaderBackground, cam, pass);
        auto shader = mShaderBackground->use();
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        shader.setTexture("uTexCubeMap", mTexSkybox);

        mMeshQuad->bind().draw();
    }

    // render terrain
    {
        for (auto const& shader : mShadersTerrain)
            setUpShader(shader.second, cam, pass);

        // render jobs
        std::map<Program*, std::map<Material const*, std::vector<VertexArray*>>> renderjobs;

        // collect meshes per material and shader
        for (auto const& chunkPair : mWorld.chunks)
        {
            if (!isVisible(*chunkPair.second))
                continue; // skip culled chunks

            for (auto const& meshPair : chunkPair.second->queryMeshes())
            {
                // create a render job for every material/mesh pair
                auto mat = mWorld.getMaterialFromIndex(meshPair.first);
                auto shader = mShadersTerrain[mat->shader].get();
                renderjobs[shader][mat].push_back(meshPair.second.get());
            }
        }

        // .. per shader
        for (auto const& shaderJobs : renderjobs)
        {
            // set up shader
            auto shader = shaderJobs.first->use();

            // .. per material
            for (auto const& matJob : shaderJobs.second)
            {
                auto mat = matJob.first;
                auto matIdx = mat->index;

                // check correct render pass
                if (matIdx < 0 && pass != RenderPass::Transparent)
                    continue;
                if (matIdx > 0 && (pass != RenderPass::Opaque && pass != RenderPass::Shadow))
                    continue;

                // set up material
                shader.setUniform("uMetallic", mat->metallic);
                shader.setUniform("uReflectivity", mat->reflectivity);
                shader.setUniform("uTextureScale", mat->textureScale);
                shader.setTexture("uTexAO", mat->texAO);
                shader.setTexture("uTexAlbedo", mat->texAlbedo);
                shader.setTexture("uTexNormal", mat->texNormal);
                shader.setTexture("uTexHeight", mat->texHeight);
                shader.setTexture("uTexRoughness", mat->texRoughness);

                // .. per mesh
                for (auto const& mesh : matJob.second)
                    mesh->bind().draw(); // render
            }
        }
    }

    // mouse hit
    if (mMouseHit.hasHit && pass == RenderPass::Opaque)
    {
        drawLine(mMouseHit.hitPos, mMouseHit.hitPos + glm::vec3(mMouseHit.hitNormal) * 0.2f, {1, 1, 1});
    }

    // render debug box overlay
    if (mMouseHit.hasHit && pass == RenderPass::Transparent)
    {
        auto shader = mShaderOverlay->use();
        GLOW_SCOPED(enable, GL_BLEND);
        GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE);

        float overlayOpacity = 0.5f;
        glm::vec3 overlayColor;
        auto boxPos = mMouseHit.blockPos;
        if (mCtrlPressed)
        {
            // Remove material
            overlayColor = glm::vec3(1, 0, 0);
        }
        else if (mShiftPressed)
        {
            // Use pipette to get material
            overlayColor = glm::vec3(0.5, 0.5, 0);
        }
        else
        {
            // Place material
            overlayColor = glm::vec3(0, 1, 0);
            boxPos += mMouseHit.hitNormal;
        }

        shader.setUniform("uPosition", glm::vec3(boxPos));
        shader.setUniform("uColor", glm::vec4(overlayColor, overlayOpacity));

        mMeshCube->bind().draw();
    }
}


void Assignment07::setUpShader(SharedProgram const& program, camera::CameraBase* cam, RenderPass pass)
{
    auto shader = program->use();

    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 proj = cam->getProjectionMatrix();

    shader.setUniform("uIsShadowPass", pass == RenderPass::Shadow);
    shader.setUniform("uIsOpaquePass", pass == RenderPass::Opaque);
    shader.setUniform("uIsTransparentPass", pass == RenderPass::Transparent);

    shader.setUniform("uShowAmbientOcclusion", mShowAmbientOcclusion);
    shader.setUniform("uShowNormal", mShowNormal);

    shader.setUniform("uView", view);
    shader.setUniform("uProj", proj);
    shader.setUniform("uInvView", inverse(view));
    shader.setUniform("uInvProj", inverse(proj));
    shader.setUniform("uCamPos", cam->getPosition());
    shader.setTexture("uCubeMap", mTexSkybox);

    shader.setUniform("uLightDir", normalize(mLightDir));
    shader.setUniform("uAmbientLight", mAmbientLight);
    shader.setUniform("uLightColor", mLightColor);
    shader.setUniform("uRuntime", (float)mRuntime);

    shader.setTexture("uShadowMap", mShadowMap);
    shader.setUniform("uShadowViewProj", mShadowCamera.getProjectionMatrix() * mShadowCamera.getViewMatrix());

    shader.setTexture("uFramebufferOpaque", mTexOpaqueColor);
    shader.setTexture("uFramebufferDepth", mTexOpaqueDepth);
}

void Assignment07::buildLineMesh()
{
    auto ab = ArrayBuffer::create();
    ab->defineAttribute<float>("aPosition");
    ab->bind().setData(std::vector<float>({0.0f, 1.0f}));
    mMeshLine = VertexArray::create(ab, GL_LINES);
}

void Assignment07::drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color)
{
    auto shader = mShaderLine->use();
    shader.setUniform("uFrom", from);
    shader.setUniform("uTo", to);
    shader.setUniform("uColor", color);

    mMeshLine->bind().draw();
}

static void TW_CALL ButtonRebuild(void* data)
{
    ((Assignment07*)data)->rebuildWorld();
}

void Assignment07::init()
{
    // limit GPU to 60 fps
    setVSync(true);

    // we don't use the GlfwApp built-in rendering
    setUseDefaultRendering(false);

    // disable built-in camera handling with left mouse button
    setUseDefaultCameraHandlingLeft(false);

    GlfwApp::init(); // Call to base GlfwApp

    auto texPath = util::pathOf(__FILE__) + "/textures/";
    auto shaderPath = util::pathOf(__FILE__) + "/shader/";
    auto meshPath = util::pathOf(__FILE__) + "/meshes/";

    // set up camera
    {
        auto cam = getCamera();
        cam->setPosition({12, 12, 12});
        cam->setTarget({0, 0, 0});
    }

    // load shaders
    {
        glow::info() << "Loading shaders";

        // pipeline
        mShaderOutput = Program::createFromFile(shaderPath + "pipeline/output");
        mShaderCopy = Program::createFromFile(shaderPath + "pipeline/copy");
        mShaderBackground = Program::createFromFile(shaderPath + "pipeline/bg");

        // objects
        mShaderOverlay = Program::createFromFile(shaderPath + "objects/overlay");
        mShaderLine = Program::createFromFile(shaderPath + "objects/line");
    }

    // shadow map
    // -> created on demand

    // rendering pipeline
    {
        mFramebufferTargets.push_back(mTexOpaqueColor = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTargets.push_back(mTexOpaqueDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32));
        mFramebufferTargets.push_back(mTexFinalColor = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTargets.push_back(mTexFinalDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32));

        mFramebufferOpaque = Framebuffer::create({{"fColor", mTexOpaqueColor}}, mTexOpaqueDepth);
        mFramebufferFinal = Framebuffer::create({{"fColor", mTexFinalColor}}, mTexFinalDepth);
    }

    // load textures
    {
        glow::info() << "Loading textures";

        mTexSkybox = TextureCubeMap::createFromData(TextureData::createFromFileCube( //
            texPath + "bg/posx.jpg",                                                 //
            texPath + "bg/negx.jpg",                                                 //
            texPath + "bg/posy.jpg",                                                 //
            texPath + "bg/negy.jpg",                                                 //
            texPath + "bg/posz.jpg",                                                 //
            texPath + "bg/negz.jpg"                                                  //
            ));

        // terrain textures are loaded in world::init (materials)
    }

    // create geometry
    {
        glow::info() << "Loading geometry";

        mMeshQuad = geometry::Quad<>().generate();
        mMeshCube = geometry::Cube<>().generate();
        buildLineMesh();
    }

    // set up tweakbar
    {
        TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &mLightDir, "group=rendering");
        TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &mAmbientLight, "group=rendering");
        TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &mLightColor, "group=rendering");
        TwAddVarRW(tweakbar(), "Back Face Culling", TW_TYPE_BOOLCPP, &mBackFaceCulling, "group=rendering");
        TwAddVarRW(tweakbar(), "Show Wireframe", TW_TYPE_BOOLCPP, &mWireframe, "group=rendering");
        TwAddVarRW(tweakbar(), "Show Normals", TW_TYPE_BOOLCPP, &mShowNormal, "group=rendering");
        TwAddVarRW(tweakbar(), "Show AO", TW_TYPE_BOOLCPP, &mShowAmbientOcclusion, "group=rendering");
        TwAddVarRW(tweakbar(), "FXAA", TW_TYPE_BOOLCPP, &mUseFXAA, "group=rendering");
        TwAddVarRW(tweakbar(), "Dithering", TW_TYPE_BOOLCPP, &mUseDithering, "group=rendering");
        TwAddVarRW(tweakbar(), "Shadows", TW_TYPE_BOOLCPP, &mEnableShadows, "group=rendering");
        TwAddVarRW(tweakbar(), "Shadow Map Size", TW_TYPE_INT32, &mShadowMapSize, "group=rendering min=512 max=4096 step=16");
        TwAddVarRW(tweakbar(), "Render Distance", TW_TYPE_FLOAT, &mRenderDistance, "group=rendering min=1 max=1000");
        TwAddButton(tweakbar(), "Rebuild World", ButtonRebuild, this, "");

        TwDefine("Tweakbar size='220 350' valueswidth=60");
    }

    // init world
    {
        glow::info() << "Init world";

        mWorld.init();
        rebuildWorld(); // initial chunks

        // create terrain shaders
        for (auto const& mat : mWorld.materialsOpaque)
            createTerrainShader(mat.shader);
        for (auto const& mat : mWorld.materialsTranslucent)
            createTerrainShader(mat.shader);
    }
}

void Assignment07::getMouseRay(glm::vec3& pos, glm::vec3& dir) const
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

void Assignment07::updateViewRay()
{
    // calculate mouse ray
    glm::vec3 pos, dir;
    getMouseRay(pos, dir);

    mMouseHit = mWorld.rayCast(pos, dir);
}

void Assignment07::createTerrainShader(std::string const& name)
{
    if (mShadersTerrain.count(name))
        return; // already added

    glow::info() << "Loading material shader " << name << ".fsh";
    auto shaderPath = util::pathOf(__FILE__) + "/shader/terrain/";
    auto program = Program::createFromFiles({shaderPath + "terrain.vsh", shaderPath + name + ".fsh"});
    mShadersTerrain[name] = program;
}

bool Assignment07::isVisible(Chunk const& c) const
{
    // For this assignment we always render all chunks
    return true;
    /*
    // check if inside render distance
    auto dis = distance(mPlayerPos * glm::vec3(1, 0, 1), c.chunkCenter() * glm::vec3(1, 0, 1)); // xz only
    if (dis - c.size * 1.9f > mRenderDistance)
        return false;

    // TODO: culling

    return true;
    */
}

void Assignment07::updateShadowMapTexture()
{
    if (mShadowMap && (int)mShadowMap->getWidth() == mShadowMapSize)
        return; // already done

    glow::info() << "Creating " << mShadowMapSize << " x " << mShadowMapSize << " shadow map";

    auto shadowColorMap = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_R8, 1);
    mShadowMap = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_DEPTH_COMPONENT32F, 1);
    {
        auto tex = mShadowMap->bind();
        tex.setMinFilter(GL_LINEAR); // no mip-maps

        // set depth compare parameters for shadow mapping
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
    }
    // add a dummy color target (in hope that some drivers calm their mammaries)
    mFramebufferShadow = Framebuffer::create({{"fColor", shadowColorMap}}, mShadowMap);
}

bool Assignment07::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    updateViewRay();

    if (mMouseHit.hasHit && action == GLFW_PRESS && button == GLFW_MOUSE_BUTTON_LEFT)
    {
        bool modified = false;
        auto bPos = mMouseHit.blockPos;
        auto blockMat = mMouseHit.block.mat;
        if (mods & GLFW_MOD_CONTROL)
        {
            mWorld.queryBlockMutable(bPos).mat = 0;
            modified = true;
            // glow::info() << "Removing material " << int(mMouseHit.block.mat) << " at " << bPos;
        }
        else if (mods & GLFW_MOD_SHIFT)
        {
            mCurrentMaterial = blockMat;
            auto matName = mWorld.getMaterialFromIndex(blockMat)->name;
            glow::info() << "Selected material is now " << matName;
        }
        else // no modifier -> add material
        {
            bPos += mMouseHit.hitNormal;
            mWorld.queryBlockMutable(bPos).mat = mCurrentMaterial;
            modified = true;
            // glow::info() << "Adding material " << int(mCurrentMaterial) << " at " << bPos;
        }

        // trigger mesh rebuilding
        if (modified)
            mWorld.markDirty(bPos, 1);

        return true;
    }

    return false;
}

bool Assignment07::onMousePosition(double x, double y)
{
    if (GlfwApp::onMousePosition(x, y))
        return true;

    updateViewRay();

    return false;
}

bool Assignment07::onKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
        mShiftPressed = action == GLFW_PRESS;

    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
        mCtrlPressed = action == GLFW_PRESS;

    updateViewRay();

    return false;
}

void Assignment07::onResize(int w, int h)
{
    GlfwApp::onResize(w, h);

    // resize framebuffer textures
    for (auto const& t : mFramebufferTargets)
        t->bind().resize(w, h);
}
