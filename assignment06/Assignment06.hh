#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/glfw/GlfwApp.hh>

enum class RenderMode
{
    Water,
    Normals,
    Checker,
    Flux,
    Divergence,
    Unsupported
};

enum class RenderPass
{
    Shadow,
    Opaque,
    Transparent
};

/**
 * Assignment06: A fluid simulation using the shallow water equations
 */
class Assignment06 : public glow::glfw::GlfwApp
{
public:
    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec3 tangent;
        glm::vec2 texCoord;

        static std::vector<glow::ArrayBufferAttribute> attributes()
        {
            return {
                {&Vertex::pos, "aPosition"},    //
                {&Vertex::normal, "aNormal"},   //
                {&Vertex::tangent, "aTangent"}, //
                {&Vertex::texCoord, "aTexCoord"},
            };
        }
    };

private: // input and camera
    glm::vec3 mLightDir = normalize(glm::vec3(-.69, .67, -.28));
    glm::vec3 mLightColor = glm::vec3(1);
    glm::vec3 mAmbientLight = glm::vec3(0.05);

private:                               // fluid - settings
    float mGravity = 9.81;             // gravity in negative y dir
    float mFluidDrag = 0.1f;           // linear drag
    float mFluidStartHeight = 2.0f;    // water height in [m]
    float mPoolHeight = 3.0f;          // pool depth in [m]
    float mFluidResolution = 1 / 4.0f; // grid resolution in [m]
    int mFluidSizeX = 64;              // grid cells in x dir
    int mFluidSizeY = 128;             // grid cells in y dir
    float mTileSize = 0.1f;            // size of a pool tile in [m]
    float mPoolBorder = 1.0f;          // pool border size in [m]

    int mSandMeshAngles = 128;
    int mSandMeshRadii = 128;

private: // fluid
    glow::SharedTextureRectangle mCurrFluidHeight;
    glow::SharedTextureRectangle mNextFluidHeight;

    glow::SharedTextureRectangle mCurrFluidFluxX;
    glow::SharedTextureRectangle mNextFluidFluxX;

    glow::SharedTextureRectangle mCurrFluidFluxY;
    glow::SharedTextureRectangle mNextFluidFluxY;

    glow::SharedFramebuffer mFluidFramebufferFluxToCurr;
    glow::SharedFramebuffer mFluidFramebufferHeightToCurr;
    glow::SharedFramebuffer mFluidFramebufferFluxToNext;
    glow::SharedFramebuffer mFluidFramebufferHeightToNext;

private: // graphics
    glow::SharedVertexArray mMeshPool;
    glow::SharedVertexArray mMeshLadder;
    glow::SharedVertexArray mMeshWater;
    glow::SharedVertexArray mMeshQuad;
    glow::SharedVertexArray mMeshFluidCell;
    glow::SharedVertexArray mMeshPalms;


    glow::SharedProgram mShaderOutput;
    glow::SharedProgram mShaderCopy;
    glow::SharedProgram mShaderBackground;
    glow::SharedProgram mShaderDunes;
    glow::SharedProgram mShaderPool;
    glow::SharedProgram mShaderPalms;
    glow::SharedProgram mShaderWater;
    glow::SharedProgram mShaderLadder;
    glow::SharedProgram mShaderFluidUpdateHeight;
    glow::SharedProgram mShaderFluidUpdateFlux;

    // Pool tiles textures
    glow::SharedTexture2D mTexPoolColor;
    glow::SharedTexture2D mTexPoolNormal;
    glow::SharedTexture2D mTexPoolRoughness;
    glow::SharedTexture2D mTexPoolHeight;
    glow::SharedTexture2D mTexPoolAO;

    // Pool ladder textures
    glow::SharedTexture2D mTexLadderColor;
    glow::SharedTexture2D mTexLadderNormal;
    glow::SharedTexture2D mTexLadderRoughness;
    glow::SharedTexture2D mTexLadderAO;

    // Sand textures
    glow::SharedTexture2D mTexSandColor;
    glow::SharedTexture2D mTexSandNormal;
    glow::SharedTexture2D mTexSandHeight;
    glow::SharedTexture2D mTexSandAO;

    // Palm textures
    glow::SharedTexture2D mTexPalmColor;
    glow::SharedTexture2D mTexPalmNormal;
    glow::SharedTexture2D mTexPalmSpecular;

    glow::SharedTexture2D mTexWaterNormal;
    glow::SharedTextureCubeMap mTexCubeMap;

    // Rendering pipeline
    std::vector<glow::SharedTextureRectangle> mFramebufferTargets;
    glow::SharedFramebuffer mFramebufferOpaque;
    glow::SharedTextureRectangle mTexOpaqueColor;
    glow::SharedTextureRectangle mTexOpaqueDepth;

    glow::SharedFramebuffer mFramebufferFinal;
    glow::SharedTextureRectangle mTexFinalColor;
    glow::SharedTextureRectangle mTexFinalDepth;

    // shadows
    const int mShadowMapSize = 1024;
    glow::SharedTexture2D mShadowMap;
    glow::SharedFramebuffer mFramebufferShadow;
    glow::camera::FixedCamera mShadowCamera;

    // show wireframe instead of filled polygons
    bool mWireframe = false;

    // Slow-down factor which is applied on delta_t
    float mSlowDown = 1.0f;

    // Total accumulated runtime
    float mRuntime = 0.0f;

    // Mouse interaction
    bool mWaterInteraction = false;
    glm::vec2 mInteractionCoords;

    // whether to render water, normals, flux, ...
    int mRenderMode = 0;

    // Fluid viscosity
    float mViscosityTau = 1.0;
    float mLinearDragTau = 5.0;

private: // helper
    /// sets common uniforms such as 3D transformation matrices
    void setUpShader(glow::SharedProgram const& program, glow::camera::CameraBase* cam, RenderPass pass);
    /// sets all fluid-related uniforms
    void setUpFluidShader(glow::SharedProgram const& program, float elapsedSeconds);

    /// builds the pool mesh
    void buildPoolMesh();

    /// get a ray from current mouse pos into the scene
    void getMouseRay(glm::vec3& pos, glm::vec3& dir);

public:
    /// resets fluid state
    void resetFluid();

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;

    void renderScene(glow::camera::CameraBase* cam, RenderPass pass);

    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onMousePosition(double x, double y) override;

    void onResize(int w, int h) override;
};
