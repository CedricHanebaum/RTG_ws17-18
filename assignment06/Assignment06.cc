#include "Assignment06.hh"

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
#include <glow-extras/geometry/Quad.hh>

// AntTweakBar
#include <AntTweakBar.h>

// GLFW
#include <GLFW/glfw3.h>

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

void Assignment06::update(float elapsedSeconds)
{
    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    mRuntime += elapsedSeconds;

    // update fluid
    {
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        // update flux
        {
            setUpFluidShader(mShaderFluidUpdateFlux, elapsedSeconds);
            auto fb = mFluidFramebufferFluxToNext->bind();
            auto shader = mShaderFluidUpdateFlux->use();
            mMeshQuad->bind().draw();
        }

        // set curr velocities to next
        swap(mCurrFluidFluxX, mNextFluidFluxX);
        swap(mCurrFluidFluxY, mNextFluidFluxY);
        swap(mFluidFramebufferFluxToCurr, mFluidFramebufferFluxToNext);

        // update heights
        {
            setUpFluidShader(mShaderFluidUpdateHeight, elapsedSeconds);
            auto fb = mFluidFramebufferHeightToNext->bind();
            auto shader = mShaderFluidUpdateHeight->use();
            mMeshQuad->bind().draw();
        }

        // set curr heights to next
        swap(mCurrFluidHeight, mNextFluidHeight);
        swap(mFluidFramebufferHeightToCurr, mFluidFramebufferHeightToNext);
    }
}

void Assignment06::render(float elapsedSeconds)
{
    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);

    // renormalize light dir (tweakbar might have changed it)
    mLightDir = normalize(mLightDir);

    // draw shadow map
    {
        auto fb = mFramebufferShadow->bind();
        glClear(GL_DEPTH_BUFFER_BIT);

        // set up shadow camera
        auto shadowCamDis = 50.0f;
        auto shadowSize = 25.0f;
        auto shadowPos = mLightDir * shadowCamDis;
        mShadowCamera.setPosition(shadowPos);
        mShadowCamera.setViewMatrix(lookAt(shadowPos, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0)));
        mShadowCamera.setProjectionMatrix(glm::ortho(-shadowSize, shadowSize, -shadowSize, shadowSize, 1.0f, shadowCamDis * 2.0f));
        mShadowCamera.setViewportSize({mShadowMapSize, mShadowMapSize});

        // render scene from light
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

        // render water part of scene
        renderScene(getCamera().get(), RenderPass::Transparent);
    }

    // draw output
    {
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        auto shader = mShaderOutput->use();
        shader.setTexture("uTexture", mTexFinalColor);

        mMeshQuad->bind().draw();
    }
}

void Assignment06::renderScene(camera::CameraBase* cam, RenderPass pass)
{
    // draw background
    if (pass == RenderPass::Opaque)
    {
        setUpShader(mShaderBackground, cam, pass);
        auto shader = mShaderBackground->use();
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        shader.setTexture("uTexCubeMap", mTexCubeMap);

        mMeshQuad->bind().draw();
    }

    // draw pool
    if (pass == RenderPass::Opaque || //
        pass == RenderPass::Shadow)
    {
        setUpShader(mShaderPool, cam, pass);
        auto shader = mShaderPool->use();
        shader.setTexture("uTexPoolColor", mTexPoolColor);
        shader.setTexture("uTexPoolNormal", mTexPoolNormal);
        shader.setTexture("uTexPoolRoughness", mTexPoolRoughness);
        shader.setTexture("uTexPoolHeight", mTexPoolHeight);
        shader.setTexture("uTexPoolAO", mTexPoolAO);
        mMeshPool->bind().draw();
    }

    // draw ladder
    if (pass == RenderPass::Opaque || //
        pass == RenderPass::Shadow)
    {
        setUpShader(mShaderLadder, cam, pass);
        auto shader = mShaderLadder->use();
        shader.setTexture("uTexLadderColor", mTexLadderColor);
        shader.setTexture("uTexLadderNormal", mTexLadderNormal);
        shader.setTexture("uTexLadderRoughness", mTexLadderRoughness);
        shader.setTexture("uTexLadderAO", mTexLadderAO);
        mMeshLadder->bind().draw();
    }

    // draw dunes
    if (pass == RenderPass::Opaque || //
        pass == RenderPass::Shadow)
    {
        setUpShader(mShaderDunes, cam, pass);
        auto shader = mShaderDunes->use();
        shader.setUniform("uCountAngles", mSandMeshAngles);
        shader.setUniform("uCountRadii", mSandMeshRadii);
        shader.setUniform("uFluidSizeX", mFluidSizeX);
        shader.setUniform("uFluidSizeY", mFluidSizeY);
        shader.setUniform("uFluidResolution", mFluidResolution);
        shader.setUniform("uPoolBorder", mPoolBorder);

        shader.setTexture("uTexSandColor", mTexSandColor);
        shader.setTexture("uTexSandNormal", mTexSandNormal);
        shader.setTexture("uTexSandHeight", mTexSandHeight);
        shader.setTexture("uTexSandAO", mTexSandAO);

        mMeshQuad->bind().draw(mSandMeshAngles * mSandMeshRadii);
    }

    // draw palm trees
    if (pass == RenderPass::Opaque || //
        pass == RenderPass::Shadow)
    {
/// Task 2.c
/// Set up the palm tree shader and render the mesh
///
/// Your job is to:
///     - call setUpShader(...) with appropriate parameters
///     - set all palm textures for the fragment shader
///     - set the uPalmPos uniform for the vertex shader
///     - render the mMeshPalms mesh with disabled back-face culling
///       (palm tree leaves should be rendered from both sides)
///
/// Notes:
///     - setting up this shader is quite similar to others
///     - the palm tree(s) should be rendered close to the pool
///     - do not forget to enable culling after rendering
///
/// ============= STUDENT CODE BEGIN =============

        setUpShader(mShaderPalms, cam, pass);
        auto shader = mShaderPalms->use();

        shader.setTexture("uTexPalmColor", mTexPalmColor);
        shader.setTexture("uTexPalmNormal", mTexPalmNormal);
        shader.setTexture("uTexPalmSpecular", mTexPalmSpecular);

        float poolSizeX = mFluidSizeX * mFluidResolution * 0.5 + mPoolBorder * 0.5;
        float poolSizeY = mFluidSizeY * mFluidResolution * 0.5 + mPoolBorder * 0.5;

        shader.setUniform("uPalmPos", glm::vec3(poolSizeX + 3, 0, poolSizeY + 3));

        GLOW_SCOPED(disable, GL_CULL_FACE);

        mMeshPalms->bind().draw(1);

/// ============= STUDENT CODE END =============
    }

    // draw water
    if (pass == RenderPass::Transparent)
    {
        setUpShader(mShaderWater, cam, pass);
        setUpFluidShader(mShaderWater, 0.0f);
        auto shader = mShaderWater->use();

        shader.setTexture("uTexWaterNormal", mTexWaterNormal);

        // draw quads via instancing
        auto quadsX = mFluidSizeX - 1;
        auto quadsY = mFluidSizeY - 1;
        mMeshFluidCell->bind().draw(quadsX * quadsY);
    }
}


void Assignment06::setUpShader(SharedProgram const& program, camera::CameraBase* cam, RenderPass pass)
{
    auto shader = program->use();

    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 proj = cam->getProjectionMatrix();

    shader.setUniform("uIsShadowPass", pass == RenderPass::Shadow);
    shader.setUniform("uIsOpaquePass", pass == RenderPass::Opaque);
    shader.setUniform("uIsTransparentPass", pass == RenderPass::Transparent);

    shader.setUniform("uView", view);
    shader.setUniform("uProj", proj);
    shader.setUniform("uInvView", inverse(view));
    shader.setUniform("uInvProj", inverse(proj));
    shader.setUniform("uCamPos", cam->getPosition());
    shader.setTexture("uCubeMap", mTexCubeMap);

    shader.setUniform("uLightDir", normalize(mLightDir));
    shader.setUniform("uAmbientLight", mAmbientLight);
    shader.setUniform("uLightColor", mLightColor);
    shader.setUniform("uRuntime", mRuntime);

    shader.setTexture("uShadowMap", mShadowMap);
    shader.setUniform("uShadowViewProj", mShadowCamera.getProjectionMatrix() * mShadowCamera.getViewMatrix());

    shader.setTexture("uFramebufferOpaque", mTexOpaqueColor);
    shader.setTexture("uFramebufferDepth", mTexOpaqueDepth);
}

void Assignment06::setUpFluidShader(const SharedProgram& program, float elapsedSeconds)
{
    auto shader = program->use();

    shader.setTexture("uFluidHeight", mCurrFluidHeight);
    shader.setTexture("uFluidFluxX", mCurrFluidFluxX);
    shader.setTexture("uFluidFluxY", mCurrFluidFluxY);

    shader.setUniform("uPoolHeight", mPoolHeight);
    shader.setUniform("uFluidDrag", mFluidDrag);
    shader.setUniform("uFluidResolution", mFluidResolution);
    shader.setUniform("uFluidSizeX", mFluidSizeX);
    shader.setUniform("uFluidSizeY", mFluidSizeY);
    shader.setUniform("uGravity", mGravity);
    shader.setUniform("uDeltaT", elapsedSeconds / mSlowDown);
    shader.setUniform("uViscosityTau", mViscosityTau);
    shader.setUniform("uLinearDragTau", mLinearDragTau);

    // water interaction
    shader.setUniform("uWaterInteraction", mWaterInteraction);
    shader.setUniform("uInteractionCoords", mInteractionCoords);

    shader.setUniform("uRenderMode", mRenderMode);
}

void Assignment06::buildPoolMesh()
{
    std::vector<Vertex> vertices;
    std::vector<uint8_t> indices;

    auto addQuad = [&](glm::vec3 start, glm::vec3 dirX, glm::vec3 dirY) {
        auto v00 = start;
        auto v01 = start + dirY;
        auto v10 = start + dirX;
        auto v11 = start + dirX + dirY;

        auto n = normalize(cross(dirY, dirX));
        auto t = normalize(dirX);

        auto bi = vertices.size();

        float distX = length(dirX);
        float distY = length(dirY);
        vertices.push_back({v00, n, t, glm::vec2(0, 0)});
        vertices.push_back({v01, n, t, glm::vec2(0, distY)});
        vertices.push_back({v10, n, t, glm::vec2(distX, 0)});
        vertices.push_back({v11, n, t, glm::vec2(distX, distY)});

        indices.push_back(bi + 0);
        indices.push_back(bi + 3);
        indices.push_back(bi + 2);

        indices.push_back(bi + 0);
        indices.push_back(bi + 1);
        indices.push_back(bi + 3);
    };

    // assemble mesh
    {
        auto poolW = mFluidSizeX * mFluidResolution;
        auto poolH = mFluidSizeY * mFluidResolution;

        // ground
        addQuad({-poolW / 2.0f, -mPoolHeight, -poolH / 2.0f}, //
                {poolW, 0, 0}, {0, 0, poolH});

        // borders
        addQuad({-poolW / 2.0f, 0, -poolH / 2.0f}, //
                {poolW, 0, 0}, {0, -mPoolHeight, 0});
        addQuad({-poolW / 2.0f, -mPoolHeight, poolH / 2.0f}, //
                {poolW, 0, 0}, {0, mPoolHeight, 0});
        addQuad({-poolW / 2.0f, 0, -poolH / 2.0f}, //
                {0, -mPoolHeight, 0}, {0, 0, poolH});
        addQuad({poolW / 2.0f, -mPoolHeight, -poolH / 2.0f}, //
                {0, mPoolHeight, 0}, {0, 0, poolH});

        // borders2
        addQuad({-poolW / 2 - mPoolBorder, 0, -poolH / 2 - mPoolBorder}, //
                {mPoolBorder, 0, 0}, {0, 0, 2 * mPoolBorder + poolH});
        addQuad({poolW / 2, 0, -poolH / 2 - mPoolBorder}, //
                {mPoolBorder, 0, 0}, {0, 0, 2 * mPoolBorder + poolH});
        addQuad({-poolW / 2, 0, -poolH / 2 - mPoolBorder}, //
                {poolW, 0, 0}, {0, 0, mPoolBorder});
        addQuad({-poolW / 2, 0, poolH / 2}, //
                {poolW, 0, 0}, {0, 0, mPoolBorder});
    }

    // create mesh
    auto ab = ArrayBuffer::create(Vertex::attributes());
    ab->bind().setData(vertices);
    auto eab = ElementArrayBuffer::create(indices);
    mMeshPool = VertexArray::create(ab, eab);
}

void Assignment06::resetFluid()
{
    // clear state
    mCurrFluidHeight->clear<float>(2.0f);
    mCurrFluidFluxX->clear<float>(0.0f);
    mCurrFluidFluxY->clear<float>(0.0f);
}

static void TW_CALL ButtonClearFluid(void* data)
{
    ((Assignment06*)data)->resetFluid();
}

void Assignment06::init()
{
    // limit GPU to 60 fps
    setVSync(true);

    // we don't use the GlfwApp built-in rendering
    setUseDefaultRendering(false);

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

        mShaderCopy = Program::createFromFile(shaderPath + "copy");
        mShaderOutput = Program::createFromFile(shaderPath + "output");
        mShaderBackground = Program::createFromFile(shaderPath + "bg");
        mShaderDunes = Program::createFromFile(shaderPath + "dunes");
/// Task 2.a
/// Load the palm tree shader from file
///
/// Your job is to:
///     - load the shader "mShaderPalms" needed to draw the palm trees
///
/// Notes:
///     - you can do it!
///
/// ============= STUDENT CODE BEGIN =============
        mShaderPalms = Program::createFromFile(shaderPath + "palms");
/// ============= STUDENT CODE END =============
        mShaderPool = Program::createFromFile(shaderPath + "pool");
        mShaderLadder = Program::createFromFile(shaderPath + "ladder");
        mShaderWater = Program::createFromFile(shaderPath + "water");
        mShaderFluidUpdateHeight = Program::createFromFiles({shaderPath + "fluid.vsh", //
                                                             shaderPath + "fluid-update-height.fsh"});
        mShaderFluidUpdateFlux = Program::createFromFiles({shaderPath + "fluid.vsh", //
                                                           shaderPath + "fluid-update-flux.fsh"});
    }

    // shadow map
    {
        mShadowMap = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_DEPTH_COMPONENT32F, 1);
        {
            auto tex = mShadowMap->bind();
            tex.setMinFilter(GL_LINEAR); // no mip-maps
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_REF_TO_TEXTURE);
            glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_FUNC, GL_LESS);
        }
        mShadowMap->bind().setMinFilter(GL_LINEAR);
        mFramebufferShadow = Framebuffer::create({}, mShadowMap); // depth-only
                                                                  // TODO: shadow sampler
    }

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

        mTexCubeMap = TextureCubeMap::createFromData(TextureData::createFromFileCube( //
            texPath + "posx.jpg",                                                     //
            texPath + "negx.jpg",                                                     //
            texPath + "posy.jpg",                                                     //
            texPath + "negy.jpg",                                                     //
            texPath + "posz.jpg",                                                     //
            texPath + "negz.jpg"                                                      //
            ));

        // Pool tiles textures
        mTexPoolColor = Texture2D::createFromFile(texPath + "pool.color.png");
        mTexPoolNormal = Texture2D::createFromFile(texPath + "pool.normal.png", ColorSpace::Linear);
        mTexPoolRoughness = Texture2D::createFromFile(texPath + "pool.roughness.png", ColorSpace::Linear);
        mTexPoolHeight = Texture2D::createFromFile(texPath + "pool.height.png", ColorSpace::Linear);
        mTexPoolAO = Texture2D::createFromFile(texPath + "pool.ao.png", ColorSpace::Linear);

        // Pool ladder textures
        mTexLadderColor = Texture2D::createFromFile(texPath + "metal.color.png");
        mTexLadderNormal = Texture2D::createFromFile(texPath + "metal.normal.png", ColorSpace::Linear);
        mTexLadderRoughness = Texture2D::createFromFile(texPath + "metal.roughness.png", ColorSpace::Linear);
        mTexLadderAO = Texture2D::createFromFile(texPath + "metal.ao.png", ColorSpace::Linear);

        // Sand textures
        mTexSandColor = Texture2D::createFromFile(texPath + "sand.color.png");
        mTexSandNormal = Texture2D::createFromFile(texPath + "sand.normal.png", ColorSpace::Linear);
        mTexSandHeight = Texture2D::createFromFile(texPath + "sand.height.png", ColorSpace::Linear);
        mTexSandAO = Texture2D::createFromFile(texPath + "sand.ao.png", ColorSpace::Linear);

        {
/// Task 2.b
/// Load the palm tree textures from disk
///
/// Your job is to:
///     - load the required palm tree textures (have a look at the shader)
///
/// Notes:
///     - pay attention to the color space parameter for the normal texture
///
/// ============= STUDENT CODE BEGIN =============
            mTexPalmColor = Texture2D::createFromFile(texPath + "palm.color.png");
            mTexPalmNormal = Texture2D::createFromFile(texPath + "palm.normal.png");
            mTexPalmSpecular = Texture2D::createFromFile(texPath + "palm.specular.png");
/// ============= STUDENT CODE END =============
        }


        // Water textures
        mTexWaterNormal = Texture2D::createFromFile(texPath + "water.normal.png", ColorSpace::Linear);
    }

    // create geometry
    {
        glow::info() << "Loading geometry";

        mMeshQuad = geometry::Quad<>().generate();

        // Fluid cell (Quad that consists of 4 triangles)
        std::vector<glm::vec2> vertices = {
            {0, 1},    //
            {0, 0},    //
            {1, 0},    //
            {1, 1},    //
            {0.5, 0.5} //
        };

        std::vector<uint8_t> indices = {
            0, 1, 4, //
            1, 2, 4, //
            2, 3, 4, //
            3, 0, 4  //
        };
        std::reverse(indices.begin(), indices.end());
        auto ab = ArrayBuffer::create();
        ab->defineAttribute<glm::vec2>("aPosition");
        ab->bind().setData(vertices);
        auto eab = ElementArrayBuffer::create(indices);
        mMeshFluidCell = VertexArray::create(ab, eab);

        buildPoolMesh();
        mMeshLadder = assimp::Importer().load(meshPath + "ladder.obj");
        mMeshPalms = assimp::Importer().load(meshPath + "palms.obj");
    }

    // set up fluid
    {
        glow::info() << "Filling pool with water";

        auto texW = mFluidSizeX + 1;
        auto texH = mFluidSizeY + 1;

        // actually sizeX x sizeY
        mCurrFluidHeight = TextureRectangle::create(texW, texH, GL_R32F);
        mNextFluidHeight = TextureRectangle::create(texW, texH, GL_R32F);

        // actually (sizeX + 1) x sizeY
        mCurrFluidFluxX = TextureRectangle::create(texW, texH, GL_R32F);
        mNextFluidFluxX = TextureRectangle::create(texW, texH, GL_R32F);

        // actually sizeX x (sizeY + 1)
        mCurrFluidFluxY = TextureRectangle::create(texW, texH, GL_R32F);
        mNextFluidFluxY = TextureRectangle::create(texW, texH, GL_R32F);

        mFluidFramebufferHeightToCurr = Framebuffer::create({
            {"fHeight", mCurrFluidHeight}, //
        });
        mFluidFramebufferHeightToNext = Framebuffer::create({
            {"fHeight", mNextFluidHeight}, //
        });
        mFluidFramebufferFluxToCurr = Framebuffer::create({
            {"fFluxX", mCurrFluidFluxX}, //
            {"fFluxY", mCurrFluidFluxY}, //
        });
        mFluidFramebufferFluxToNext = Framebuffer::create({
            {"fFluxX", mNextFluidFluxX}, //
            {"fFluxY", mNextFluidFluxY}, //
        });

        resetFluid();
    }

    // set up tweakbar
    {
        TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &mLightDir, "group=rendering");
        TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &mAmbientLight, "group=rendering");
        TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &mLightColor, "group=rendering");
        TwAddVarRW(tweakbar(), "Wireframe", TW_TYPE_BOOLCPP, &mWireframe, "group=rendering");
        TwAddVarRW(tweakbar(), "SlowDown Factor", TW_TYPE_FLOAT, &mSlowDown, "group=rendering min=1");
        TwAddVarRW(tweakbar(), "Viscosity (tau)", TW_TYPE_FLOAT, &mViscosityTau, "group=rendering min=0.001 max=10.0 step=0.1");
        TwAddVarRW(tweakbar(), "Linear Drag (tau)", TW_TYPE_FLOAT, &mLinearDragTau, "group=rendering min=0.001 max=10.0 step=0.1");

        TwEnumVal renderModeEV[] = {
            {(int)RenderMode::Water, "Water"},           //
            {(int)RenderMode::Normals, "Normals"},       //
            {(int)RenderMode::Checker, "Checker"},       //
            {(int)RenderMode::Flux, "Flux"},             //
            {(int)RenderMode::Divergence, "Divergence"}, //
        };
        TwType coloringType = TwDefineEnum("Render Mode", renderModeEV, 5);
        TwAddVarRW(tweakbar(), "Render Mode", coloringType, &mRenderMode, "group=rendering");

        TwAddButton(tweakbar(), "Reset Fluid", ButtonClearFluid, this, "group=scene");

        TwDefine("Tweakbar size='220 220' valueswidth=60");
    }
}

void Assignment06::getMouseRay(glm::vec3& pos, glm::vec3& dir)
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

bool Assignment06::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    return onMousePosition(x, y);
}

bool Assignment06::onMousePosition(double x, double y)
{
    if (TwEventMousePosGLFW(window(), x, y))
        return true;

    if (isMouseButtonPressed(GLFW_MOUSE_BUTTON_LEFT))
    {
        // calculate mouse ray
        glm::vec3 pos, dir;
        getMouseRay(pos, dir);

        glm::vec3 planePos = {0, 0, 0};
        glm::vec3 planeNormal = {0, 1, 0};
        float denom = dot(planeNormal, dir);

        glm::vec3 rayresult = pos + dot(planePos - pos, planeNormal) / denom * dir;
        float h = mFluidSizeY * mFluidResolution;
        float w = mFluidSizeX * mFluidResolution;
        mInteractionCoords = glm::vec2(rayresult.x, rayresult.z) / glm::vec2(w, h) + glm::vec2(0.5);
        mInteractionCoords *= glm::vec2(mFluidSizeX, mFluidSizeY);
        mWaterInteraction = true;

        return true;
    }
    else if (mWaterInteraction)
    {
        mWaterInteraction = false;
        return true;
    }

    return GlfwApp::onMousePosition(x, y);
}

void Assignment06::onResize(int w, int h)
{
    GlfwApp::onResize(w, h);

    // resize framebuffer textures
    for (auto const& t : mFramebufferTargets)
        t->bind().resize(w, h);
}
