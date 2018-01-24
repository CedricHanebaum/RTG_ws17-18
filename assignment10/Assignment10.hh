#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/glfw/GlfwApp.hh>

#include "Character.hh"
#include "Chunk.hh"
#include "Material.hh"
#include "World.hh"

enum class RenderPass
{
    Shadow,
    DepthPre,
    Opaque,
    Transparent
};

enum class DebugTarget
{
    Output,

    OpaqueDepth,
    ShadedOpaque,

    GBufferAlbedo,
    GBufferAO,
    GBufferNormal,
    GBufferMetallic,
    GBufferRoughness,
    GBufferTranslucency,

    TBufferColor,
    TBufferAlpha,
    TBufferDistortion,
    TBufferBlurriness,

    ShadowCascade0,
    ShadowCascade1,
    ShadowCascade2,

    HDR,
    HDRBrightExtract,
    HDRBloomA,
    HDRBloomB,
    HDRLDR,
};

/**
 * Assignment10: A minecraft clone (part 3)
 */
class Assignment10 : public glow::glfw::GlfwApp
{
private:
    /// our block world
    World mWorld;

    /// our character moving through the world
    Character mCharacter;

private: // input and camera
    glm::vec3 mLightDir = normalize(glm::vec3(.25, .66, .75));
    glm::vec3 mLightColor = glm::vec3(1);
    glm::vec3 mAmbientLight = glm::vec3(0.05);

    // "Player"
    glm::vec3 mPlayerPos = {0, 0, 0};

    // Interaction
    RayHit mMouseHit;
    bool mShiftPressed = false;
    bool mCtrlPressed = false;
    int8_t mCurrentMaterial = 1;
    bool mFreeFlightCamera = false;
    bool mDoJump = true;

    // Lights
    float mLightSpawnCountdown = 0.0f;

    struct LightSource
    {
        glm::vec3 position;
        float radius;
        glm::vec3 velocity;
        float intensity; // keep real unit a secret
        glm::vec3 color;
        int seed;
    };
    std::vector<LightSource> mLightSources;

private: // object gfx
    // terrain
    std::map<std::string, glow::SharedProgram> mShadersTerrain;
    glow::SharedProgram mShaderTerrainShadow;
    glow::SharedProgram mShaderTerrainDepthPre;

    // background
    glow::SharedTextureCubeMap mTexSkybox;

    // objects
    glow::SharedProgram mShaderLineTransparent;
    glow::SharedProgram mShaderPlants;
    glow::SharedTexture2D mTexPlants;

    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mMeshLine;

    // lights
    glow::SharedProgram mShaderLightSprites;
    glow::SharedVertexArray mMeshLightSpheres;
    glow::SharedVertexArray mMeshLightSprites;
    glow::SharedArrayBuffer mLightArrayBuffer;
    glow::SharedTexture2D mTexLightSprites;

private: // rendering pipeline
    struct RenderTarget
    {
        glow::SharedTextureRectangle target;
        int downsampled;

        RenderTarget(glow::SharedTextureRectangle target, int downsampled = 0)
          : target(target), downsampled(downsampled)
        {
        }
    };

    std::vector<RenderTarget> mFramebufferTargets;

    // meshes
    glow::SharedVertexArray mMeshQuad;

    // shaders
    glow::SharedProgram mShaderOutput;
    glow::SharedProgram mShaderClear;
    glow::SharedProgram mShaderShadowBlurX;
    glow::SharedProgram mShaderShadowBlurY;
    glow::SharedProgram mShaderFullscreenLight;
    glow::SharedProgram mShaderPointLight;
    glow::SharedProgram mShaderTransparentResolve;
    glow::SharedProgram mShaderDownsample;
    glow::SharedProgram mShaderScreenspaceReflections;
    glow::SharedProgram mShaderBrightExtract;
    glow::SharedProgram mShaderBloomDownsample;
    glow::SharedProgram mShaderBloomKawase;
    glow::SharedProgram mShaderToneMapping;

    // Shadow Pass
    int mShadowMapSize = 1024;
    struct ShadowCascade
    {
        glow::SharedFramebuffer framebuffer;
        glow::camera::FixedCamera camera;
        float minRange = -1;
        float maxRange = -1;
    };
    glow::SharedTexture2DArray mShadowMaps;
    std::vector<ShadowCascade> mShadowCascades;
    glm::mat4 mShadowViewProjs[SHADOW_CASCADES];
    glm::vec3 mShadowPos;
    float mShadowExponent = 80.0f;
    float mShadowRange = 200.0f;
    glow::SharedFramebuffer mFramebufferShadowBlur;
    glow::SharedTexture2D mShadowBlurTarget;

    // Depth Pre-Pass
    glow::SharedFramebuffer mFramebufferDepthPre;
    glow::SharedTextureRectangle mTexOpaqueDepth;

    // Opaque Pass
    glow::SharedFramebuffer mFramebufferGBuffer;
    glow::SharedTextureRectangle mTexGBufferMatA;  // normals, metallic
    glow::SharedTextureRectangle mTexGBufferMatB;  // roughness, translucency SSS
    glow::SharedTextureRectangle mTexGBufferColor; // albedo, AO

    // Light Pass
    glow::SharedFramebuffer mFramebufferShadedOpaque;
    glow::SharedTextureRectangle mTexShadedOpaque;

    // Downsampling, Screen Space Reflections and Godrays Pass
    glow::SharedFramebuffer mFramebufferGBufferDownsampled;
    glow::SharedTextureRectangle mTexOpaqueDepthDownsampled;
    glow::SharedTextureRectangle mTexGBufferMatADownsampled;
    glow::SharedTextureRectangle mTexGBufferMatBDownsampled;
    glow::SharedTextureRectangle mTexShadedOpaqueDownsampled;

    glow::SharedFramebuffer mFramebufferSSR;
    glow::SharedTextureRectangle mTexSSR;

    // Transparent Pass
    glow::SharedFramebuffer mFramebufferTBuffer;
    glow::SharedTextureRectangle mTexTBufferAccumA;
    glow::SharedTextureRectangle mTexTBufferAccumB;
    glow::SharedTextureRectangle mTexTBufferDistortion; // offset + LOD bias

    // Transparent Resolve
    glow::SharedFramebuffer mFramebufferTransparentResolve;
    glow::SharedTextureRectangle mTexHDRColor;

    // HDR Pass
    glow::SharedFramebuffer mFramebufferBrightExtract;
    glow::SharedFramebuffer mFramebufferBloomToA;
    glow::SharedFramebuffer mFramebufferBloomToB;
    glow::SharedFramebuffer mFramebufferLDRColor;
    glow::SharedTextureRectangle mTexBrightExtract;
    glow::SharedTextureRectangle mTexBloomDownsampledA;
    glow::SharedTextureRectangle mTexBloomDownsampledB;
    glow::SharedTextureRectangle mTexLDRColor;
    void swapBloomTargets();

    // Output Stage
    // .. no FBO (to screen)
    DebugTarget mDebugOutput = DebugTarget::Output;

    // gfx options
    bool mUseFXAA = true;
    bool mUseDithering = true;
    bool mEnableShadows = true;
    bool mSoftShadows = true;
    bool mShowWireframeOpaque = false;
    bool mShowWireframeTransparent = false;
    bool mShowDebugLights = false;
    bool mDrawBackground = true;
    bool mShowSSR = false;

    // tone mapping
    float mToneMappingA = 1.0f;
    float mToneMappingGamma = 1.0f;
    float mBloomStrength = 1.0f;
    float mBloomThreshold = 1.0f;
    bool mBloom = true;

    // pipeline
    bool mEnablePointLights = true;
    bool mPassDepthPre = true;
    bool mPassOpaque = true;
    bool mPassTransparent = true;

    // culling
    float mRenderDistance = 32;
    bool mEnableCustomBFC = true;
    bool mEnableFrustumCulling = true;

    // debug
    bool mBackFaceCulling = true;

    // stats
    int mStatsChunksGenerated = -1;
    int mStatsMeshesRendered[4];
    int mStatsVerticesRendered[4];
    float mStatsVerticesPerMesh[4];

private: // gfx options
    /// accumulated time
    double mRuntime = 0.0f;

private: // helper
    /// sets common uniforms such as 3D transformation matrices
    void setUpShader(glow::Program* program, glow::camera::CameraBase* cam, RenderPass pass);
    void setUpLightShader(glow::Program* program, glow::camera::CameraBase* cam);

    /// get a ray from current mouse pos into the scene
    void getMouseRay(glm::vec3& pos, glm::vec3& dir) const;

    /// performs a raycast and updates selected block
    void updateViewRay();

    /// Creates a shader for terrain (with terrain.vsh)
    /// Also registers it with mShadersTerrain
    /// Does not create the same shader twice
    void createTerrainShader(std::string const& name);

    /// Updates shadow map texture if size changed
    void updateShadowMapTexture();

    /// Registers tweakbar entries
    void setUpTweakBar();

    // line drawing
    void buildLineMesh();
    void drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color, RenderPass pass);
    void drawAABB(glm::vec3 min, glm::vec3 max, glm::vec3 color, RenderPass pass);

    /// spawn a light source close to the player
    void spawnLightSource(const glm::vec3& origin);
    /// update all light sources
    void updateLightSources(float elapsedSeconds);
    /// draw a sphere / light source debug object
    void drawSphere(glm::vec3 pos, float radius, glm::vec3 color);

private: // rendering
    /// renders the scene for a render pass
    void renderScene(glow::camera::CameraBase* cam, RenderPass pass);

    // pipeline passes
    void renderShadowPass();
    void renderDepthPrePass();
    void renderOpaquePass();
    void renderLightPass();
    void renderSSRPass();
    void renderTransparentPass();
    void renderTransparentResolve();
    void renderHDRPass();
    void renderOutputStage();

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;

    // Input/Event handling
    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onMousePosition(double x, double y) override;
    bool onKey(int key, int scancode, int action, int mods) override;
    void onResize(int w, int h) override;
};
