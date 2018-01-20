#include "Assignment09.hh"

// System headers
#include <cstdint>
#include <fstream>
#include <vector>

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/profiling.hh>
#include <glow/common/scoped_gl.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/Framebuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/Texture2DArray.hh>
#include <glow/objects/TextureCubeMap.hh>
#include <glow/objects/TextureRectangle.hh>
#include <glow/objects/VertexArray.hh>

#include <glow/data/TextureData.hh>

#include <glow-extras/assimp/Importer.hh>
#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/camera/GenericCamera.hh>
#include <glow-extras/geometry/Cube.hh>
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/geometry/UVSphere.hh>

// AntTweakBar
#include <AntTweakBar.h>

// GLFW
#include <GLFW/glfw3.h>

#include "FrustumCuller.hh"

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

namespace
{
float randomFloat(float minV, float maxV)
{
    return minV + (maxV - minV) * float(rand()) / float(RAND_MAX);
}
}

void Assignment09::update(float elapsedSeconds)
{
    mLightSpawnCountdown -= elapsedSeconds;

    if (mLightSpawnCountdown < 0.0f)
    {
        for (auto const& chunkPair : mWorld.chunks)
        {
            auto const& chunk = chunkPair.second;

            // Do not evaluate chunks that are further away than render distance
            // (0.5 * sqrt(3) ~ 0.9)
            float maxDist = mRenderDistance + 0.9 * CHUNK_SIZE;
            float maxDist2 = maxDist * maxDist;
            if (glm::distance2(getCamera()->getPosition(), chunk->chunkCenter()) > maxDist2)
                continue;

            // Iterate over all light fountains and spawn light sources
            for (auto& lF : chunk->getActiveLightFountains())
            {
                // Center light pos in block
                glm::vec3 lFpos = glm::vec3(lF) + 0.5f;

                // Do not spawn if out of render distance
                if (glm::distance2(glm::vec3(lFpos), getCamera()->getPosition()) > mRenderDistance * mRenderDistance)
                    continue;

                spawnLightSource(lFpos);
            }
        }
        // Reset countdown to some random amount of seconds
        mLightSpawnCountdown = randomFloat(0.5, 2) * 0.1;
    }

    updateLightSources(elapsedSeconds);


    if (mFreeFlightCamera)
    {
        setCameraMoveSpeed(15.0f);
        mCharacter.setPosition(getCamera()->getPosition());
    }
    else
        setCameraMoveSpeed(0.0f);

    GlfwApp::update(elapsedSeconds); // Call to base GlfwApp

    mRuntime += elapsedSeconds;

    // generate chunks that might be visible
    mWorld.notifyCameraPosition(getCamera()->getPosition(), mRenderDistance);

    // update terrain
    mWorld.update(elapsedSeconds);

    // character
    if (!mFreeFlightCamera)
    {
        auto walkspeed = mCharacter.getMovementSpeed(mShiftPressed);

        auto movement = glm::vec3(0, 0, 0);
        if (isKeyPressed(GLFW_KEY_W)) // forward
            movement.z -= walkspeed;
        if (isKeyPressed(GLFW_KEY_S)) // backward
            movement.z += walkspeed;
        if (isKeyPressed(GLFW_KEY_A)) // left
            movement.x -= walkspeed;
        if (isKeyPressed(GLFW_KEY_D)) // right
            movement.x += walkspeed;
        if (mDoJump)
        {
            movement.y += mCharacter.getJumpSpeed(mShiftPressed);
            mDoJump = false;
        }

        mCharacter.update(mWorld, elapsedSeconds, movement);
    }
}

void Assignment09::render(float elapsedSeconds)
{
    GLOW_SCOPED(enable, GL_DEPTH_TEST);
    GLOW_SCOPED(enable, GL_CULL_FACE);
    if (!mBackFaceCulling)
        glDisable(GL_CULL_FACE);

    // update stats
    mStatsChunksGenerated = mWorld.chunks.size();
    for (auto i = 0; i < 4; ++i)
    {
        mStatsMeshesRendered[i] = 0;
        mStatsVerticesRendered[i] = 0;
    }

    // update camera
    getCamera()->setFarClippingPlane(mRenderDistance);

    // build lights
    {
        std::vector<LightVertex> lightData;
        lightData.reserve(mLightSources.size());
        for (auto const& l : mLightSources)
            lightData.push_back({l.position, l.radius, l.color, l.seed});
        mLightArrayBuffer->bind().setData(lightData);
    }

    // renormalize light dir (tweakbar might have changed it)
    mLightDir = normalize(mLightDir);

    // rendering pipeline
    {
        // Shadow Pass
        renderShadowPass();

        // Depth Pre-Pass
        renderDepthPrePass();

        // Opaque Pass
        renderOpaquePass();

        // Light Pass
        renderLightPass();

        // SSR, Godrays
        // (work in progress)
        // renderSSRPass();

        // Transparent Pass
        renderTransparentPass();

        // Transparent Resolve
        renderTransparentResolve();

        // HDR, Bloom, Tone Mapping
        renderHDRPass();

        // Output Stage
        renderOutputStage();
    }

    // update stats
    for (auto i = 0; i < 4; ++i)
        mStatsVerticesPerMesh[i] = mStatsVerticesRendered[i] == 0 ? -1 : //
                                       mStatsVerticesRendered[i] / (float)mStatsMeshesRendered[i];
}

void Assignment09::renderShadowPass()
{
    // ensure that sizes are correct
    updateShadowMapTexture();

    auto cam = getCamera();
    for (auto cascIdx = 0; cascIdx < SHADOW_CASCADES; ++cascIdx)
    {
        auto& cascade = mShadowCascades[cascIdx];

        // build frustum part
        cascade.minRange = mRenderDistance * (cascIdx + 0.0f) / SHADOW_CASCADES;
        cascade.maxRange = mRenderDistance * (cascIdx + 1.0f) / SHADOW_CASCADES;

        auto cInvView = inverse(cam->getViewMatrix());
        auto cInvProj = inverse(cam->getProjectionMatrix());

        auto sView = lookAt(mLightDir, glm::vec3(0, 0, 0), glm::vec3(0, 1, 0));

        auto sMin = glm::vec3(std::numeric_limits<float>::max());
        auto sMax = -sMin;

        // calculate shadow frustum
        auto const res = 8;
        for (auto nz : {-1, 1})
            for (auto iy = 0; iy < res; ++iy)
                for (auto ix = 0; ix < res; ++ix)
                {
                    auto ny = iy / (res - 1.0f) * 2 - 1;
                    auto nx = ix / (res - 1.0f) * 2 - 1;

                    auto ndc = glm::vec4(nx, ny, nz, 1.0f);
                    auto viewPos = cInvProj * ndc;
                    viewPos /= viewPos.w;
                    auto worldPos = glm::vec3(cInvView * viewPos);

                    auto dir = glm::vec3(worldPos - cam->getPosition());
                    auto dis = length(dir);
                    worldPos = cam->getPosition() + dir * (glm::clamp(dis, cascade.minRange, cascade.maxRange) / dis);

                    auto shadowViewPos = glm::vec3(sView * glm::vec4(worldPos, 1.0));

                    sMin = min(sMin, shadowViewPos);
                    sMax = max(sMax, shadowViewPos);
                }

        // elongate shadow aabb
        sMin -= 1.0f;
        sMax += 1.0f;
        sMax.z = sMin.z + glm::max(mShadowRange, sMax.z - sMin.z);

        // min..max -> 0..1 -> -1..1
        auto sProj = scale(glm::vec3(1, 1, -1)) *        // flip z for BFC
                     translate(glm::vec3(-1.0f)) *       //
                     scale(1.0f / (sMax - sMin) * 2.0) * //
                     translate(-sMin);

        // TODO: stabilize min/max

        // set up shadow camera
        cascade.camera.setPosition(mLightDir);
        cascade.camera.setViewMatrix(sView);
        cascade.camera.setProjectionMatrix(sProj);
        cascade.camera.setViewportSize({mShadowMapSize, mShadowMapSize});
        mShadowViewProjs[cascIdx] = cascade.camera.getProjectionMatrix() * cascade.camera.getViewMatrix();

        // render shadowmap
        {
            auto fb = cascade.framebuffer->bind();
            glClear(GL_DEPTH_BUFFER_BIT);

            // clear sm
            {
                GLOW_SCOPED(disable, GL_DEPTH_TEST);
                GLOW_SCOPED(disable, GL_CULL_FACE);
                auto shader = mShaderClear->use();
                shader.setUniform("uColor", glm::vec4(glm::exp(mShadowExponent)));
                mMeshQuad->bind().draw();
            }

            // render scene from light
            if (mEnableShadows)
                renderScene(&cascade.camera, RenderPass::Shadow);
        }

        // blur shadow map for soft shadows
        if (mEnableShadows && mSoftShadows)
        {
            GLOW_SCOPED(disable, GL_DEPTH_TEST);
            GLOW_SCOPED(disable, GL_CULL_FACE);
            auto vao = mMeshQuad->bind();

            // blur x
            {
                auto fb = mFramebufferShadowBlur->bind();
                auto shader = mShaderShadowBlurX->use();
                shader.setTexture("uTexture", mShadowMaps);
                shader.setUniform("uCascade", cascIdx);

                vao.draw();
            }

            mShadowBlurTarget->setMipmapsGenerated(true); // only one LOD level

            // blur y
            {
                auto fb = cascade.framebuffer->bind();
                auto shader = mShaderShadowBlurY->use();
                shader.setTexture("uTexture", mShadowBlurTarget);

                vao.draw();
            }
        }
    }
}

void Assignment09::renderDepthPrePass()
{
    auto fb = mFramebufferDepthPre->bind();

    // clear depth
    glClear(GL_DEPTH_BUFFER_BIT);
    GLOW_SCOPED(depthFunc, GL_LESS);

    // render scene depth-pre
    if (mPassDepthPre)
        renderScene(getCamera().get(), RenderPass::DepthPre);
}

void Assignment09::renderOpaquePass()
{
    // debug: wireframe rendering
    GLOW_SCOPED(polygonMode, GL_FRONT_AND_BACK, mShowWireframeOpaque ? GL_LINE : GL_FILL);

    auto fb = mFramebufferGBuffer->bind();

    // clear the g buffer
    GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);      // NO DEPTH CLEAR!
    GLOW_SCOPED(depthFunc, GL_LEQUAL); // <=
    GLOW_SCOPED(depthMask, GL_FALSE);  // for this assignment, we disable depth writing to see Depth Pre-Pass usage

    // sRGB write
    GLOW_SCOPED(enable, GL_FRAMEBUFFER_SRGB);

    // render scene normally
    if (mPassOpaque)
        renderScene(getCamera().get(), RenderPass::Opaque);
}

void Assignment09::renderLightPass()
{
    setUpLightShader(mShaderFullscreenLight.get(), getCamera().get());
    setUpLightShader(mShaderPointLight.get(), getCamera().get());

    auto fb = mFramebufferShadedOpaque->bind();

    GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT); // NO DEPTH CLEAR!

    // additive blending
    GLOW_SCOPED(enable, GL_BLEND);
    GLOW_SCOPED(blendFunc, GL_ONE, GL_ONE);

    // full-screen (directional + ambient + bg)
    {
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        GLOW_SCOPED(disable, GL_CULL_FACE);

        auto shader = mShaderFullscreenLight->use();

        mMeshQuad->bind().draw();
    }

    // point lights
    if (mEnablePointLights)
    {
        // debug: wireframe rendering
        GLOW_SCOPED(polygonMode, GL_FRONT_AND_BACK, mShowDebugLights ? GL_LINE : GL_FILL);

        // we explicitly want culling
        GLOW_SCOPED(enable, GL_CULL_FACE);

        // depth TEST yes, depth WRITE no
        GLOW_SCOPED(enable, GL_DEPTH_TEST);
        GLOW_SCOPED(depthMask, GL_FALSE);

        // draw lights
        auto shader = mShaderPointLight->use();
        mMeshLightSpheres->bind().draw();
    }
}

void Assignment09::renderSSRPass()
{
    if (!mShowSSR)
    {
        // Assure the (additively blended) SSR target is black
        GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 0.0f);
        auto fb = mFramebufferSSR->bind();
        glClear(GL_COLOR_BUFFER_BIT);
        return;
    }

    setUpLightShader(mShaderDownsample.get(), getCamera().get());
    setUpLightShader(mShaderScreenspaceReflections.get(), getCamera().get());


    GLOW_SCOPED(disable, GL_CULL_FACE);

    // Downsample
    {
        GLOW_SCOPED(enable, GL_DEPTH_TEST);
        GLOW_SCOPED(depthMask, GL_TRUE);
        GLOW_SCOPED(depthFunc, GL_ALWAYS);

        // TODO: multiple downsample iterations?
        auto fb = mFramebufferGBufferDownsampled->bind();

        GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        auto shader = mShaderDownsample->use();
        shader.setTexture("uTexShadedOpaque", mTexShadedOpaque);

        mMeshQuad->bind().draw();
    }


    // SSR / God Rays
    GLOW_SCOPED(disable, GL_DEPTH_TEST);
    {
        auto fb = mFramebufferSSR->bind();

        glm::mat4 viewToPixel = getCamera()->getProjectionMatrix();

        auto texW = mTexShadedOpaqueDownsampled->getWidth();
        auto texH = mTexShadedOpaqueDownsampled->getHeight();

        viewToPixel = translate(glm::vec3(1, 1, 1)) * viewToPixel;
        viewToPixel = scale(glm::vec3(0.5, 0.5, 0.5)) * viewToPixel;
        viewToPixel = scale(glm::vec3(texW, texH, 1.0)) * viewToPixel;

        auto shader = mShaderScreenspaceReflections->use();
        shader.setTexture("uTexShadedOpaque", mTexShadedOpaqueDownsampled);
        shader.setTexture("uTexGBufferMatA", mTexGBufferMatADownsampled);
        shader.setTexture("uTexGBufferMatB", mTexGBufferMatBDownsampled);
        shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepthDownsampled);
        shader.setUniform("uViewToPixel", viewToPixel);

        auto cNear = getCamera()->getNearClippingPlane();
        auto cFar = getCamera()->getFarClippingPlane();
        auto clipInfo = glm::vec3(cNear * cFar, cNear - cFar, cFar);
        shader.setUniform("uClipInfo", clipInfo);

        mMeshQuad->bind().draw();
    }
}

void Assignment09::renderTransparentPass()
{
    // debug: wireframe rendering
    GLOW_SCOPED(polygonMode, GL_FRONT_AND_BACK, mShowWireframeTransparent ? GL_LINE : GL_FILL);

    auto fb = mFramebufferTBuffer->bind();

    // clear t-buffer
    GLOW_SCOPED(clearColor, 0.0f, 0.0f, 0.0f, 1.0f); // revealage is in alpha channel
    glClear(GL_COLOR_BUFFER_BIT);                    // NO DEPTH CLEAR!

    // special blending
    GLOW_SCOPED(enable, GL_BLEND);
    glBlendFuncSeparate(GL_ONE, GL_ONE, GL_ZERO, GL_ONE_MINUS_SRC_ALPHA);

    // no culling
    GLOW_SCOPED(disable, GL_CULL_FACE);

    // no depth write
    GLOW_SCOPED(depthMask, GL_FALSE);

    // render translucent part of scene (e.g. water)
    if (mPassTransparent)
        renderScene(getCamera().get(), RenderPass::Transparent);
}

void Assignment09::renderTransparentResolve()
{
    auto fb = mFramebufferTransparentResolve->bind();

    GLOW_SCOPED(disable, GL_DEPTH_TEST);
    GLOW_SCOPED(disable, GL_CULL_FACE);

    setUpLightShader(mShaderTransparentResolve.get(), getCamera().get());
    auto shader = mShaderTransparentResolve->use();
    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setTexture("uTexShadedOpaque", mTexShadedOpaque);
    shader.setTexture("uTexTBufferAccumA", mTexTBufferAccumA);
    shader.setTexture("uTexTBufferAccumB", mTexTBufferAccumB);
    shader.setTexture("uTexTBufferDistortion", mTexTBufferDistortion);
    // WIP: shader.setTexture("uTexSSR", mTexSSR);

    shader.setUniform("uDrawBackground", mDrawBackground);

    mMeshQuad->bind().draw();
}

void Assignment09::renderHDRPass()
{
    /// Your job is to implement the Tone Mapping + Bloom Pass
    /// This pass happens after Transparent Resolve and before Output
    /// Output now uses mTexLDRColor (still 16 bit and linear)
    /// (You should be familiar with the overall architecture by now,
    ///  so this assignment is less "filling in the blanks")
    ///
    /// Overview:
    ///     - Bright Extract Step
    ///       (from HDR Color to Bright Extract texture)
    ///
    ///     - Downsample Step
    ///       (from Bright Extract to one of the downsampled bloom textures)
    ///
    ///     - Blur
    ///       (a "01223" Kawase blur on the downsampled bloom textures)
    ///
    ///     - Tone Mapping + Bloom
    ///       (applying tone mapping to HDR color and adding the blurred bloom)
    ///
    /// Potentially relevant textures:
    ///     - mTexHDRColor
    ///     - mTexLDRColor
    ///     - mTexBrightExtract
    ///     - mTexBloomDownsampledA (half width and height)
    ///     - mTexBloomDownsampledB (half width and height)
    ///
    /// ... with associated framebuffers:
    ///     - mFramebufferBrightExtract
    ///     - mFramebufferBloomToA
    ///     - mFramebufferBloomToB
    ///     - mFramebufferLDRColor
    ///
    /// There is a new function "swapBloomTargets()"
    /// that swaps the bloom targets A and B (textures and framebuffers).
    /// This can be used to conveniently implement the blur.
    ///
    /// Relevant shaders:
    ///     - mShaderBrightExtract (fullscreen.bright-extract.fsh)
    ///     - mShaderBloomDownsample (fullscreen.bloom-downsample.fsh)
    ///     - mShaderBloomKawase (fullscreen.bloom-kawase.fsh)
    ///     - mShaderToneMapping (fullscreen.tone-mapping.fsh)
    ///
    /// Relevant variables (for uniforms):
    ///     - mBloom (flag: if false, bloom should be disabled by using zero strength in shader)
    ///     - mBloomStrength
    ///     - mBloomThreshold
    ///     - mToneMappingA
    ///     - mToneMappingGamma
    ///
    /// Step details:
    ///     - Bright Extract Step:
    ///         - We "extract" brightness with the following formula:
    ///           c' = max(c * A - T, 0)
    ///           b = c' / (c' + 1)
    ///           where
    ///             c - is the HDR input color
    ///             b - is the extracted brightness
    ///             A - is the scaling from the tone mapping operator
    ///             T - is the bloom threshold
    ///           this function extracts useful colors in dark and bright scenarios
    ///           and limits the result to [0..1] to prevent artifacts by ultra bright pixels
    ///     - Downsampling:
    ///         - Simple downsampling by averaging
    ///     - Kawase Blur:
    ///         - Swap between the two downsampled blur targets while applying the "01223" kawase blur
    ///         - `for (float distance : {0, 1, 2, 2, 3})` might help
    ///         - consult the slides (or internet) for the correct pixel offsets
    ///           (double check that you sample at the correct locations
    ///            such that your 4 samples per pass actually use 16 values in total)
    ///     - Tone Mapping:
    ///         - We use the global tone mapping operator "gamma compression"
    ///           (c' = A * c^gamma)
    ///         - Bloom is added _after_ tone mapping is applied
    ///         - Bloom is scaled by uBloomStrength
    ///         - Note that the bloom texture is downsampled
    ///           (and should be upsampled using hardware bilinear interpolation)
    ///
    /// Debugging:
    ///     - the AntTweakBar has a setting "Output" that allows you to see intermediate targets
    ///       you can use this to debug your HDR pass
    ///       (especially the last 5 entries are interesting for you)
    ///
    /// Misc Notes:
    ///     - remember that you are not allowed to read and write to the same texture in a single pass
    ///     - you also have to write the shader code
    ///
    /// ============= STUDENT CODE BEGIN =============


    /// ============= STUDENT CODE END =============
}

void Assignment09::renderOutputStage()
{
    GLOW_SCOPED(disable, GL_DEPTH_TEST);
    GLOW_SCOPED(disable, GL_CULL_FACE);

    // upload shader debug info
    setUpShader(mShaderOutput.get(), getCamera().get(), RenderPass::Transparent);

    auto shader = mShaderOutput->use();
    shader.setTexture("uTexture", mTexLDRColor);
    shader.setUniform("uUseFXAA", mUseFXAA);
    shader.setUniform("uUseDithering", mUseDithering);

    // pipeline debug
    shader.setTexture("uShadowMaps", mShadowMaps);
    shader.setUniform("uShadowExponent", mShadowExponent);

    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setTexture("uTexShadedOpaque", mTexShadedOpaque);

    shader.setTexture("uTexGBufferColor", mTexGBufferColor);
    shader.setTexture("uTexGBufferMatA", mTexGBufferMatA);
    shader.setTexture("uTexGBufferMatB", mTexGBufferMatB);

    shader.setTexture("uTexTBufferAccumA", mTexTBufferAccumA);
    shader.setTexture("uTexTBufferAccumB", mTexTBufferAccumB);
    shader.setTexture("uTexTBufferDistortion", mTexTBufferDistortion);

    shader.setTexture("uTexHDRColor", mTexHDRColor);
    shader.setTexture("uTexBrightExtract", mTexBrightExtract);
    shader.setTexture("uTexBloomA", mTexBloomDownsampledA);
    shader.setTexture("uTexBloomB", mTexBloomDownsampledB);

    shader.setUniform("uDebugOutput", (int)mDebugOutput);

    mMeshQuad->bind().draw();
}

void Assignment09::renderScene(camera::CameraBase* cam, RenderPass pass)
{
    // set up general purpose shaders
    switch (pass)
    {
    case RenderPass::Transparent:
        setUpShader(mShaderLineTransparent.get(), cam, pass);
        setUpShader(mShaderLightSprites.get(), cam, pass);
        break;
    default:
        break;
    }

    // render terrain
    {
        FrustumCuller culler(*cam, pass == RenderPass::Shadow);

        struct RenderJob
        {
            Program* program;
            RenderMaterial const* mat;
            VertexArray* mesh;
            float camDis;
        };

        // render jobs
        std::vector<RenderJob> jobs;

        // collect meshes per material and shader
        {
            for (auto const& chunkPair : mWorld.chunks)
            {
                auto const& chunk = chunkPair.second;

                // early out
                if (chunk->isFullyAir())
                    continue; // CAUTION: fully solid is wrong here -> top layer may have faces!

                // view-frustum culling
                if (mEnableFrustumCulling && !culler.isAabbVisible(chunk->getAabbMin(), chunk->getAabbMax()))
                    continue; // skip culled chunks

                // render distance
                if (pass != RenderPass::Shadow && !culler.isAabbInRange(chunk->getAabbMin(), chunk->getAabbMax(), mRenderDistance))
                    continue; // not in range

                for (auto const& mesh : chunk->queryMeshes())
                {
                    // check correct render pass
                    auto mat = mesh.mat.get();
                    if (!mat->opaque && pass != RenderPass::Transparent)
                        continue;
                    if (mat->opaque && (pass != RenderPass::Opaque && pass != RenderPass::Shadow && pass != RenderPass::DepthPre))
                        continue;

                    // view-frustum culling (pt. 2)
                    if (mEnableFrustumCulling && !culler.isAabbVisible(mesh.aabbMin, mesh.aabbMax))
                        continue; // skip culled meshes

                    // custom BFC
                    if (mEnableCustomBFC && mat->opaque && !culler.isFaceVisible(mesh.dir, mesh.aabbMin, mesh.aabbMax))
                        continue;

                    // render distance
                    if (pass != RenderPass::Shadow && !culler.isAabbInRange(mesh.aabbMin, mesh.aabbMax, mRenderDistance))
                        continue; // not in range

                    // create a render job for every material/mesh pair
                    Program* shader = nullptr;
                    VertexArray* vao = nullptr;
                    switch (pass)
                    {
                    case RenderPass::Shadow:
                        vao = mesh.vaoPosOnly.get();
                        shader = mShaderTerrainShadow.get();
                        break;

                    case RenderPass::DepthPre:
                        vao = mesh.vaoPosOnly.get();
                        shader = mShaderTerrainDepthPre.get();
                        break;

                    case RenderPass::Transparent:
                    case RenderPass::Opaque:
                        vao = mesh.vaoFull.get();
                        shader = mShadersTerrain[mat->shader].get();
                        break;

                    default:
                        assert(0 && "not supported");
                        break;
                    }

                    auto camDis = distance(cam->getPosition(), (mesh.aabbMin + mesh.aabbMax) / 2.0);

                    // add render job
                    jobs.push_back({shader, mat, vao, camDis});
                }
            }
        }

        // .. sort renderjobs
        {
            // sort by
            // .. shader
            // .. material
            // .. front-to-back
            std::sort(jobs.begin(), jobs.end(), [](RenderJob const& a, RenderJob const& b) {
                if (a.program != b.program)
                    return a.program < b.program;
                if (a.mat != b.mat)
                    return a.mat < b.mat;
                return a.camDis < b.camDis;
            });
        }

        // .. render per shader
        {
            auto idxShader = 0u;
            while (idxShader < jobs.size())
            {
                // set up shader
                auto program = jobs[idxShader].program;
                setUpShader(program, cam, pass);
                auto shader = program->use();

                // .. per material
                auto idxMaterial = idxShader;
                while (idxMaterial < jobs.size() && jobs[idxMaterial].program == program)
                {
                    auto mat = jobs[idxMaterial].mat;

                    // set up material
                    shader.setUniform("uMetallic", mat->metallic);
                    shader.setUniform("uReflectivity", mat->reflectivity);
                    shader.setUniform("uTranslucency", mat->translucency);
                    shader.setUniform("uTextureScale", mat->textureScale);
                    shader.setTexture("uTexAO", mat->texAO);
                    shader.setTexture("uTexAlbedo", mat->texAlbedo);
                    shader.setTexture("uTexNormal", mat->texNormal);
                    shader.setTexture("uTexHeight", mat->texHeight);
                    shader.setTexture("uTexRoughness", mat->texRoughness);

                    // .. per mesh
                    auto idxMesh = idxMaterial;
                    while (idxMesh < jobs.size() && jobs[idxMesh].mat == mat)
                    {
                        auto mesh = jobs[idxMesh].mesh;

                        // keep stats
                        mStatsMeshesRendered[(int)pass]++;
                        mStatsVerticesRendered[(int)pass] += mesh->getVertexCount();

                        mesh->bind().draw(); // render

                        // advance idx
                        ++idxMesh;
                    }

                    // advance idx
                    idxMaterial = idxMesh;
                }

                // advance idx
                idxShader = idxMaterial;
            }
        }
    }

    // mouse hit
    if (mMouseHit.hasHit && pass == RenderPass::Transparent)
    {
        drawLine(mMouseHit.hitPos, mMouseHit.hitPos + glm::vec3(mMouseHit.hitNormal) * 0.2f, {1, 1, 1}, pass);
    }

    // lights
    if (mEnablePointLights && pass == RenderPass::Transparent)
    {
        GLOW_SCOPED(disable, GL_CULL_FACE); // no culling

        auto shader = mShaderLightSprites->use();
        shader.setTexture("uTexLightSprites", mTexLightSprites);

        mMeshLightSprites->bind().draw();
    }

    // render debug box overlay
    if (mMouseHit.hasHit && pass == RenderPass::Transparent)
    {
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

        // draw AABB
        GLOW_SCOPED(disable, GL_DEPTH_TEST);
        drawAABB(glm::vec3(boxPos) + 0.01f, glm::vec3(boxPos + 1) - 0.01f, overlayColor, pass);
    }
}

void Assignment09::swapBloomTargets()
{
    std::swap(mTexBloomDownsampledA, mTexBloomDownsampledB);
    std::swap(mFramebufferBloomToA, mFramebufferBloomToB);
}

void Assignment09::setUpShader(glow::Program* program, camera::CameraBase* cam, RenderPass pass)
{
    auto shader = program->use();

    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 proj = cam->getProjectionMatrix();

    shader.setUniform("uView", view);
    shader.setUniform("uProj", proj);
    shader.setUniform("uViewProj", proj * view);
    shader.setUniform("uInvView", inverse(view));
    shader.setUniform("uInvProj", inverse(proj));
    shader.setUniform("uCamPos", cam->getPosition());

    shader.setUniform("uRuntime", (float)mRuntime);

    shader.setUniform("uShowWrongDepthPre", mShowWrongDepthPre);

    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setUniform("uRenderDistance", mRenderDistance);

    if (pass == RenderPass::Transparent)
    {
        shader.setTexture("uTexCubeMap", mTexSkybox);
        shader.setUniform("uLightDir", normalize(mLightDir));
        shader.setUniform("uAmbientLight", mAmbientLight);
        shader.setUniform("uLightColor", mLightColor);

        shader.setUniform("uShadowExponent", mShadowExponent);
        shader.setTexture("uShadowMaps", mShadowMaps);
        shader.setUniform("uShadowViewProjs", mShadowViewProjs);
        shader.setUniform("uShadowPos", mShadowPos);
        shader.setUniform("uShadowRange", mShadowRange);
    }

    if (pass == RenderPass::Shadow)
    {
        shader.setUniform("uShadowExponent", mShadowExponent);
        shader.setUniform("uShadowPos", mShadowPos);
    }
}

void Assignment09::setUpLightShader(glow::Program* program, glow::camera::CameraBase* cam)
{
    auto shader = program->use();

    glm::mat4 view = cam->getViewMatrix();
    glm::mat4 proj = cam->getProjectionMatrix();

    shader.setUniform("uDebugLights", mShowDebugLights);

    shader.setUniform("uView", view);
    shader.setUniform("uProj", proj);
    shader.setUniform("uViewProj", proj * view);
    shader.setUniform("uInvView", inverse(view));
    shader.setUniform("uInvProj", inverse(proj));
    shader.setUniform("uCamPos", cam->getPosition());

    shader.setTexture("uTexCubeMap", mTexSkybox);
    shader.setUniform("uLightDir", normalize(mLightDir));
    shader.setUniform("uAmbientLight", mAmbientLight);
    shader.setUniform("uLightColor", mLightColor);
    shader.setUniform("uRenderDistance", mRenderDistance);

    shader.setUniform("uShadowExponent", mShadowExponent);
    shader.setTexture("uShadowMaps", mShadowMaps);
    shader.setUniform("uShadowViewProjs", mShadowViewProjs);
    shader.setUniform("uShadowPos", mShadowPos);
    shader.setUniform("uShadowRange", mShadowRange);

    shader.setTexture("uTexOpaqueDepth", mTexOpaqueDepth);
    shader.setTexture("uTexGBufferColor", mTexGBufferColor);
    shader.setTexture("uTexGBufferMatA", mTexGBufferMatA);
    shader.setTexture("uTexGBufferMatB", mTexGBufferMatB);
}

void Assignment09::buildLineMesh()
{
    auto ab = ArrayBuffer::create();
    ab->defineAttribute<float>("aPosition");
    ab->bind().setData(std::vector<float>({0.0f, 1.0f}));
    mMeshLine = VertexArray::create(ab, GL_LINES);
}

void Assignment09::drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color, RenderPass pass)
{
    if (pass != RenderPass::Transparent)
    {
        glow::error() << "not implemented.";
        return;
    }

    auto shader = mShaderLineTransparent->use();
    shader.setUniform("uFrom", from);
    shader.setUniform("uTo", to);
    shader.setUniform("uColor", color);

    mMeshLine->bind().draw();
}

void Assignment09::drawAABB(glm::vec3 min, glm::vec3 max, glm::vec3 color, RenderPass pass)
{
    if (pass != RenderPass::Transparent)
    {
        glow::error() << "not implemented.";
        return;
    }

    auto shader = mShaderLineTransparent->use();
    auto vao = mMeshLine->bind();

    shader.setUniform("uColor", color);

    for (auto dir : {0, 1, 2})
        for (auto dx : {0, 1})
            for (auto dy : {0, 1})
            {
                glm::vec3 n(dir == 0, dir == 1, dir == 2);
                glm::vec3 t(dir == 1, dir == 2, dir == 0);
                glm::vec3 b(dir == 2, dir == 0, dir == 1);

                auto s = t * dx + b * dy;
                auto e = s + n;

                shader.setUniform("uFrom", mix(min, max, s));
                shader.setUniform("uTo", mix(min, max, e));

                vao.draw();
            }
}

void Assignment09::spawnLightSource(glm::vec3 const& origin)
{
    LightSource ls;
    ls.position = origin;
    ls.radius = randomFloat(0.8, 2.5);
    ls.velocity = glm::vec3(randomFloat(-.6, .6), randomFloat(4.0, 6.5), randomFloat(-.6, .6));
    ls.intensity = 1.0;
    ls.seed = rand();
    ls.color = rgbColor(glm::vec3(randomFloat(0, 360), 1, 1));
    mLightSources.push_back(ls);
}

void Assignment09::updateLightSources(float elapsedSeconds)
{
    // gravity would be too much
    float acceleration = -3.0;
    for (auto i = int(mLightSources.size()) - 1; i >= 0; --i)
    {
        auto& ls = mLightSources[i];
        ls.velocity += glm::vec3(0, acceleration, 0) * elapsedSeconds;
        ls.position += ls.velocity * elapsedSeconds;

        // Check if below terrain
        auto block = mWorld.queryBlock(ls.position + glm::vec3(0, ls.radius, 0));
        if (!block.isAir())
        {
            mLightSources.erase(mLightSources.begin() + i);
        }
    }
}

void Assignment09::init()
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

    // set up camera and character
    {
        auto cam = getCamera();
        cam->setPosition({12, 12, 12});
        cam->setTarget({0, 0, 0});

        mCharacter = Character(cam);
    }

    // load shaders
    {
        glow::info() << "Loading shaders";

        // pipeline
        mShaderOutput = Program::createFromFile(shaderPath + "pipeline/fullscreen.output");
        mShaderClear = Program::createFromFile(shaderPath + "pipeline/fullscreen.clear");
        mShaderShadowBlurX = Program::createFromFile(shaderPath + "pipeline/fullscreen.shadow-blur-x");
        mShaderShadowBlurY = Program::createFromFile(shaderPath + "pipeline/fullscreen.shadow-blur-y");
        mShaderTransparentResolve = Program::createFromFile(shaderPath + "pipeline/fullscreen.transparent-resolve");
        mShaderDownsample = Program::createFromFile(shaderPath + "pipeline/fullscreen.downsample");
        mShaderScreenspaceReflections = Program::createFromFile(shaderPath + "pipeline/fullscreen.ssr");
        mShaderBrightExtract = Program::createFromFile(shaderPath + "pipeline/fullscreen.bright-extract");
        mShaderBloomDownsample = Program::createFromFile(shaderPath + "pipeline/fullscreen.bloom-downsample");
        mShaderBloomKawase = Program::createFromFile(shaderPath + "pipeline/fullscreen.bloom-kawase");
        mShaderToneMapping = Program::createFromFile(shaderPath + "pipeline/fullscreen.tone-mapping");

        // lights
        mShaderFullscreenLight = Program::createFromFile(shaderPath + "pipeline/fullscreen.light");
        mShaderPointLight = Program::createFromFile(shaderPath + "pipeline/point-light");
        mShaderLightSprites = Program::createFromFile(shaderPath + "objects/light-sprite");

        // objects
        mShaderLineTransparent = Program::createFromFile(shaderPath + "objects/line.transparent");

        // terrain
        mShaderTerrainShadow = Program::createFromFile(shaderPath + "terrain/terrain.shadow");
        mShaderTerrainDepthPre = Program::createFromFile(shaderPath + "terrain/terrain.depth-pre");
        // ... more in world mat init
    }

    // shadow map
    // -> created on demand

    // rendering pipeline
    {
        // targets
        mFramebufferTargets.push_back(mTexOpaqueDepth = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32));
        mFramebufferDepthPre = Framebuffer::create({}, mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexGBufferColor = TextureRectangle::create(1, 1, GL_SRGB8_ALPHA8));
        mFramebufferTargets.push_back(mTexGBufferMatA = TextureRectangle::create(1, 1, GL_RGBA8));
        mFramebufferTargets.push_back(mTexGBufferMatB = TextureRectangle::create(1, 1, GL_RG8));
        mFramebufferGBuffer = Framebuffer::create(
            {
                {"fColor", mTexGBufferColor}, //
                {"fMatA", mTexGBufferMatA},   //
                {"fMatB", mTexGBufferMatB}    //
            },
            mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexShadedOpaque = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferShadedOpaque = Framebuffer::create({{"fColor", mTexShadedOpaque}}, mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexTBufferAccumA = TextureRectangle::create(1, 1, GL_RGBA16F));
        mFramebufferTargets.push_back(mTexTBufferAccumB = TextureRectangle::create(1, 1, GL_R16F));
        mFramebufferTargets.push_back(mTexTBufferDistortion = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTBuffer = Framebuffer::create(
            {
                {"fAccumA", mTexTBufferAccumA},        //
                {"fAccumB", mTexTBufferAccumB},        //
                {"fDistortion", mTexTBufferDistortion} //
            },
            mTexOpaqueDepth);

        mFramebufferTargets.push_back(mTexHDRColor = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTransparentResolve = Framebuffer::create({{"fColor", mTexHDRColor}});

        // Downsampled fb textures
        mFramebufferTargets.push_back({mTexOpaqueDepthDownsampled = TextureRectangle::create(1, 1, GL_DEPTH_COMPONENT32), 1});
        mFramebufferTargets.push_back({mTexGBufferMatADownsampled = TextureRectangle::create(1, 1, GL_RGBA8), 1});
        mFramebufferTargets.push_back({mTexGBufferMatBDownsampled = TextureRectangle::create(1, 1, GL_RG8), 1});
        mFramebufferTargets.push_back({mTexShadedOpaqueDownsampled = TextureRectangle::create(1, 1, GL_RGBA8), 1});

        mFramebufferGBufferDownsampled = Framebuffer::create(
            {
                {"fColor", mTexShadedOpaqueDownsampled}, //
                {"fMatA", mTexGBufferMatADownsampled},   //
                {"fMatB", mTexGBufferMatBDownsampled}    //
            },
            mTexOpaqueDepthDownsampled);

        mFramebufferTargets.push_back({mTexSSR = TextureRectangle::create(1, 1, GL_RGB8), 1});
        mFramebufferSSR = Framebuffer::create({{"fSSR", mTexSSR}});

        // HDR
        mFramebufferTargets.push_back(mTexBrightExtract = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTargets.push_back(mTexLDRColor = TextureRectangle::create(1, 1, GL_RGB16F));
        mFramebufferTargets.push_back({mTexBloomDownsampledA = TextureRectangle::create(1, 1, GL_RGB16F), 1});
        mFramebufferTargets.push_back({mTexBloomDownsampledB = TextureRectangle::create(1, 1, GL_RGB16F), 1});
        mFramebufferBrightExtract = Framebuffer::create({{"fColor", mTexBrightExtract}});
        mFramebufferBloomToA = Framebuffer::create({{"fColor", mTexBloomDownsampledA}});
        mFramebufferBloomToB = Framebuffer::create({{"fColor", mTexBloomDownsampledB}});
        mFramebufferLDRColor = Framebuffer::create({{"fColor", mTexLDRColor}});
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

        mTexLightSprites = Texture2D::createFromFile(texPath + "lights.jpg");

        // terrain textures are loaded in world::init (materials)
    }

    // create geometry
    {
        glow::info() << "Loading geometry";

        mMeshQuad = geometry::Quad<>().generate();
        mMeshCube = geometry::Cube<>().generate();
        buildLineMesh();
    }

    // create light geometry
    {
        mLightArrayBuffer = ArrayBuffer::create(LightVertex::attributes());
        mLightArrayBuffer->setDivisor(1); // instancing

        mMeshLightSpheres = geometry::UVSphere<>().generate();
        mMeshLightSpheres->bind().attach(mLightArrayBuffer);

        mMeshLightSprites = geometry::Quad<>().generate();
        mMeshLightSprites->bind().attach(mLightArrayBuffer);
    }

    // set up tweakbar
    setUpTweakBar();

    // init world
    {
        glow::info() << "Init world";

        mWorld.init();

        // create terrain shaders
        for (auto const& mat : mWorld.materialsOpaque)
            for (auto const& rmat : mat.renderMaterials)
                createTerrainShader(rmat->shader);

        for (auto const& mat : mWorld.materialsTranslucent)
            for (auto const& rmat : mat.renderMaterials)
                createTerrainShader(rmat->shader);
    }


    // Bilinear Sampling
    for (auto const& t : mFramebufferTargets)
    {
        auto boundTex = t.target->bind();
        boundTex.setMagFilter(GL_LINEAR);
        boundTex.setMinFilter(GL_LINEAR);
    }
}

void Assignment09::getMouseRay(glm::vec3& pos, glm::vec3& dir) const
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

void Assignment09::updateViewRay()
{
    // calculate mouse ray
    glm::vec3 pos, dir;
    getMouseRay(pos, dir);

    mMouseHit = mWorld.rayCast(pos, dir);
}

void Assignment09::createTerrainShader(std::string const& name)
{
    if (mShadersTerrain.count(name))
        return; // already added

    glow::info() << "Loading material shader " << name << ".fsh";
    auto shaderPath = util::pathOf(__FILE__) + "/shader/terrain/";
    auto program = Program::createFromFile(shaderPath + "terrain." + name);
    mShadersTerrain[name] = program;
}

void Assignment09::updateShadowMapTexture()
{
    if (mShadowMaps && (int)mShadowMaps->getWidth() == mShadowMapSize)
        return; // already done

    glow::info() << "Creating " << mShadowMapSize << " x " << mShadowMapSize << " shadow maps";

    mShadowCascades.resize(SHADOW_CASCADES);
    auto shadowDepth = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_DEPTH_COMPONENT32, 1);
    mShadowMaps = Texture2DArray::createStorageImmutable(mShadowMapSize, mShadowMapSize, SHADOW_CASCADES, GL_R32F, 1);
    mShadowMaps->bind().setMinFilter(GL_LINEAR);                     // no mip-maps
    mShadowMaps->bind().setWrap(GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE); // clamp

    for (auto i = 0; i < SHADOW_CASCADES; ++i)
    {
        auto& cascade = mShadowCascades[i];

        // attach i-th layer of mShadowMaps
        cascade.framebuffer = Framebuffer::create({{"fShadow", mShadowMaps, 0, i}}, shadowDepth);
    }

    // shadow blur texture/target
    mShadowBlurTarget = Texture2D::createStorageImmutable(mShadowMapSize, mShadowMapSize, GL_R32F, 1);
    mFramebufferShadowBlur = Framebuffer::create({{"fShadow", mShadowBlurTarget}});
}

bool Assignment09::onMouseButton(double x, double y, int button, int action, int mods, int clickCount)
{
    if (GlfwApp::onMouseButton(x, y, button, action, mods, clickCount))
        return true;

    updateViewRay();

    if (mMouseHit.hasHit && action != GLFW_RELEASE && button == GLFW_MOUSE_BUTTON_LEFT)
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

bool Assignment09::onMousePosition(double x, double y)
{
    if (GlfwApp::onMousePosition(x, y))
        return true;

    updateViewRay();

    return false;
}

bool Assignment09::onKey(int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_LEFT_SHIFT || key == GLFW_KEY_RIGHT_SHIFT)
        mShiftPressed = action != GLFW_RELEASE;

    if (key == GLFW_KEY_LEFT_CONTROL || key == GLFW_KEY_RIGHT_CONTROL)
        mCtrlPressed = action != GLFW_RELEASE;

    if (key == GLFW_KEY_F && action == GLFW_PRESS)
    {
        mFreeFlightCamera = !mFreeFlightCamera;
        glow::info() << "Set camera to " << (mFreeFlightCamera ? "free" : "first-person");
    }

    if (key == GLFW_KEY_SPACE && action == GLFW_PRESS)
        mDoJump = true;


    updateViewRay();

    return false;
}

void Assignment09::onResize(int w, int h)
{
    GlfwApp::onResize(w, h);

    // resize framebuffer textures
    for (auto const& t : mFramebufferTargets)
        t.target->bind().resize(w >> t.downsampled, h >> t.downsampled);
}

void Assignment09::setUpTweakBar()
{
    TwAddVarRW(tweakbar(), "Light Dir", TW_TYPE_DIR3F, &mLightDir, "group=rendering");
    TwAddVarRW(tweakbar(), "Ambient Light", TW_TYPE_COLOR3F, &mAmbientLight, "group=rendering");
    TwAddVarRW(tweakbar(), "Light Color", TW_TYPE_COLOR3F, &mLightColor, "group=rendering");
    TwAddVarRW(tweakbar(), "FXAA", TW_TYPE_BOOLCPP, &mUseFXAA, "group=rendering");
    TwAddVarRW(tweakbar(), "Dithering", TW_TYPE_BOOLCPP, &mUseDithering, "group=rendering");
    TwAddVarRW(tweakbar(), "Shadows", TW_TYPE_BOOLCPP, &mEnableShadows, "group=rendering");
    TwAddVarRW(tweakbar(), "Soft Shadows", TW_TYPE_BOOLCPP, &mSoftShadows, "group=rendering");
    TwAddVarRW(tweakbar(), "Shadow Max Distance", TW_TYPE_FLOAT, &mShadowRange, "group=rendering min=20 max=500");
    TwAddVarRW(tweakbar(), "Shadow Map Size", TW_TYPE_INT32, &mShadowMapSize, "group=rendering min=128 max=4096 step=16");
    TwAddVarRW(tweakbar(), "Render Background", TW_TYPE_BOOLCPP, &mDrawBackground, "group=rendering");
    // TwAddVarRW(tweakbar(), "Screen Space Reflections", TW_TYPE_BOOLCPP, &mShowSSR, "group=rendering");

    TwAddVarRW(tweakbar(), "Tone Mapping: A", TW_TYPE_FLOAT, &mToneMappingA, "group=tone-mapping min=0.01 max=10 step=0.01");
    TwAddVarRW(tweakbar(), "Tone Mapping: gamma", TW_TYPE_FLOAT, &mToneMappingGamma, "group=tone-mapping min=0.01 max=5 step=0.01");
    TwAddVarRW(tweakbar(), "Bloom", TW_TYPE_BOOLCPP, &mBloom, "group=tone-mapping");
    TwAddVarRW(tweakbar(), "Bloom Strength", TW_TYPE_FLOAT, &mBloomStrength, "group=tone-mapping min=0 max=10 step=0.1");
    TwAddVarRW(tweakbar(), "Bloom Threshold", TW_TYPE_FLOAT, &mBloomThreshold, "group=tone-mapping min=0 max=10 step=0.1");

    TwAddVarRW(tweakbar(), "Point Lights", TW_TYPE_BOOLCPP, &mEnablePointLights, "group=pipeline");
    TwAddVarRW(tweakbar(), "Pass: Depth-Pre", TW_TYPE_BOOLCPP, &mPassDepthPre, "group=pipeline");
    TwAddVarRW(tweakbar(), "Pass: Opaque", TW_TYPE_BOOLCPP, &mPassOpaque, "group=pipeline");
    TwAddVarRW(tweakbar(), "Pass: Transparent", TW_TYPE_BOOLCPP, &mPassTransparent, "group=pipeline");

    TwAddVarRW(tweakbar(), "Back Face Culling", TW_TYPE_BOOLCPP, &mBackFaceCulling, "group=debug");
    TwAddVarRW(tweakbar(), "Wireframe Opaque", TW_TYPE_BOOLCPP, &mShowWireframeOpaque, "group=debug");
    TwAddVarRW(tweakbar(), "Wireframe Transparent", TW_TYPE_BOOLCPP, &mShowWireframeTransparent, "group=debug");
    TwAddVarRW(tweakbar(), "Debug Lights", TW_TYPE_BOOLCPP, &mShowDebugLights, "group=debug");
    TwAddVarRW(tweakbar(), "Hightling Wrong Z-Pre", TW_TYPE_BOOLCPP, &mShowWrongDepthPre, "group=debug");

    TwAddVarRW(tweakbar(), "Render Distance", TW_TYPE_FLOAT, &mRenderDistance, "group=culling min=1 max=1000");
    TwAddVarRW(tweakbar(), "Frustum Culling", TW_TYPE_BOOLCPP, &mEnableFrustumCulling, "group=culling");
    TwAddVarRW(tweakbar(), "Custom BFC", TW_TYPE_BOOLCPP, &mEnableCustomBFC, "group=culling");

    TwAddVarRO(tweakbar(), "# Chunks", TW_TYPE_INT32, &mStatsChunksGenerated, "group=stats");
    TwAddVarRO(tweakbar(), "Z-Pre: Meshes", TW_TYPE_INT32, &mStatsMeshesRendered[(int)RenderPass::DepthPre], "group=stats");
    TwAddVarRO(tweakbar(), "Z-Pre: Vertices", TW_TYPE_INT32, &mStatsVerticesRendered[(int)RenderPass::DepthPre], "group=stats");
    TwAddVarRO(tweakbar(), "Z-Pre: Vertices / Mesh", TW_TYPE_FLOAT, &mStatsVerticesPerMesh[(int)RenderPass::DepthPre], "group=stats");
    TwAddVarRO(tweakbar(), "Opaque: Meshes", TW_TYPE_INT32, &mStatsMeshesRendered[(int)RenderPass::Opaque], "group=stats");
    TwAddVarRO(tweakbar(), "Opaque: Vertices", TW_TYPE_INT32, &mStatsVerticesRendered[(int)RenderPass::Opaque], "group=stats");
    TwAddVarRO(tweakbar(), "Opaque: Vertices / Mesh", TW_TYPE_FLOAT, &mStatsVerticesPerMesh[(int)RenderPass::Opaque], "group=stats");
    TwAddVarRO(tweakbar(), "Transp: Meshes", TW_TYPE_INT32, &mStatsMeshesRendered[(int)RenderPass::Transparent], "group=stats");
    TwAddVarRO(tweakbar(), "Transp: Vertices", TW_TYPE_INT32, &mStatsVerticesRendered[(int)RenderPass::Transparent], "group=stats");
    TwAddVarRO(tweakbar(), "Transp: Vertices / Mesh", TW_TYPE_FLOAT, &mStatsVerticesPerMesh[(int)RenderPass::Transparent], "group=stats");
    TwAddVarRO(tweakbar(), "Shadow: Meshes", TW_TYPE_INT32, &mStatsMeshesRendered[(int)RenderPass::Shadow], "group=stats");
    TwAddVarRO(tweakbar(), "Shadow: Vertices", TW_TYPE_INT32, &mStatsVerticesRendered[(int)RenderPass::Shadow], "group=stats");
    TwAddVarRO(tweakbar(), "Shadow: Vertices / Mesh", TW_TYPE_FLOAT, &mStatsVerticesPerMesh[(int)RenderPass::Shadow], "group=stats");

    // debug target
    TwEnumVal targetsEV[] = {
        {(int)DebugTarget::Output, "Output"}, //

        {(int)DebugTarget::OpaqueDepth, "Opaque Depth"},   //
        {(int)DebugTarget::ShadedOpaque, "Shaded Opaque"}, //

        {(int)DebugTarget::GBufferAlbedo, "G-Buffer: Albedo"},             //
        {(int)DebugTarget::GBufferAO, "G-Buffer: AO"},                     //
        {(int)DebugTarget::GBufferNormal, "G-Buffer: Normal"},             //
        {(int)DebugTarget::GBufferMetallic, "G-Buffer: Metallic"},         //
        {(int)DebugTarget::GBufferRoughness, "G-Buffer: Roughness"},       //
        {(int)DebugTarget::GBufferTranslucency, "G-Buffer: Translucency"}, //

        {(int)DebugTarget::TBufferColor, "T-Buffer: Color"},           //
        {(int)DebugTarget::TBufferAlpha, "T-Buffer: Alpha"},           //
        {(int)DebugTarget::TBufferDistortion, "T-Buffer: Distortion"}, //
        {(int)DebugTarget::TBufferBlurriness, "T-Buffer: Blurriness"}, //

        {(int)DebugTarget::ShadowCascade0, "Shadow Cascade 0"}, //
        {(int)DebugTarget::ShadowCascade1, "Shadow Cascade 1"}, //
        {(int)DebugTarget::ShadowCascade2, "Shadow Cascade 2"}, //

        {(int)DebugTarget::HDR, "HDR Color"},                        //
        {(int)DebugTarget::HDRBrightExtract, "HDR: Bright Extract"}, //
        {(int)DebugTarget::HDRBloomA, "HDR: Bloom A"},               //
        {(int)DebugTarget::HDRBloomB, "HDR: Bloom B"},               //
        {(int)DebugTarget::HDRLDR, "LDR Color"},                     //
    };
    TwAddVarRW(tweakbar(), "Output", TwDefineEnum("DebugOutput", targetsEV, 21), &mDebugOutput, "group=debug");

    TwDefine("Tweakbar size='260 650' valueswidth=60 refresh=0.1");
}
