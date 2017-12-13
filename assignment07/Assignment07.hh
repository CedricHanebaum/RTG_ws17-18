#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include <glow/objects/ArrayBufferAttribute.hh>

#include <glow-extras/camera/FixedCamera.hh>
#include <glow-extras/glfw/GlfwApp.hh>

#include "Chunk.hh"
#include "Material.hh"
#include "World.hh"

enum class RenderPass
{
    Shadow,
    Opaque,
    Transparent
};

/**
 * Assignment07: A minecraft clone
 */
class Assignment07 : public glow::glfw::GlfwApp
{
private:
    /// our block world
    World mWorld;

private: // input and camera
    glm::vec3 mLightDir = normalize(glm::vec3(.05, .66, .75));
    glm::vec3 mLightColor = glm::vec3(1);
    glm::vec3 mAmbientLight = glm::vec3(0.05);

    // "Player"
    glm::vec3 mPlayerPos = {0, 0, 0};

    // Interaction
    RayHit mMouseHit;
    bool mShiftPressed = false;
    bool mCtrlPressed = false;
    int8_t mCurrentMaterial = 1;

private: // object gfx
    // terrain
    std::map<std::string, glow::SharedProgram> mShadersTerrain;

    // background
    glow::SharedProgram mShaderBackground;
    glow::SharedTextureCubeMap mTexSkybox;

    // objects
    glow::SharedProgram mShaderOverlay;
    glow::SharedProgram mShaderLine;

    glow::SharedVertexArray mMeshCube;
    glow::SharedVertexArray mMeshLine;

private: // rendering pipeline
    std::vector<glow::SharedTextureRectangle> mFramebufferTargets;

    glow::SharedVertexArray mMeshQuad;

    glow::SharedProgram mShaderOutput;
    glow::SharedProgram mShaderCopy;

    glow::SharedFramebuffer mFramebufferOpaque;
    glow::SharedTextureRectangle mTexOpaqueColor;
    glow::SharedTextureRectangle mTexOpaqueDepth;

    glow::SharedFramebuffer mFramebufferFinal;
    glow::SharedTextureRectangle mTexFinalColor;
    glow::SharedTextureRectangle mTexFinalDepth;

    bool mUseFXAA = true;
    bool mUseDithering = true;
    bool mShowAmbientOcclusion = false;
    bool mEnableShadows = true;

    float mRenderDistance = 16;

private: // shadows
    int mShadowMapSize = 1024;
    glow::SharedTexture2D mShadowMap;
    glow::SharedFramebuffer mFramebufferShadow;
    glow::camera::FixedCamera mShadowCamera;

private: // gfx options
    /// show wireframe instead of filled polygons
    bool mWireframe = false;

    /// enable back face culling
    bool mBackFaceCulling = true;

    /// shows normal after normal mapping
    bool mShowNormal = false;

    /// accumulated time
    double mRuntime = 0.0f;

private: // helper
    /// sets common uniforms such as 3D transformation matrices
    void setUpShader(glow::SharedProgram const& program, glow::camera::CameraBase* cam, RenderPass pass);

    /// get a ray from current mouse pos into the scene
    void getMouseRay(glm::vec3& pos, glm::vec3& dir) const;

    /// performs a raycast and updates selected block
    void updateViewRay();

    /// Creates a shader for terrain (with terrain.vsh)
    /// Also registers it with mShadersTerrain
    /// Does not create the same shader twice
    void createTerrainShader(std::string const& name);

    /// Returns true iff a given chunk should be rendered ("is visible")
    bool isVisible(Chunk const& c) const;

    /// Updates shadow map texture if size changed
    void updateShadowMapTexture();

    // line drawing
    void buildLineMesh();
    void drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color);

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;

    /// clears all chunks and rebuilds the world
    void rebuildWorld();

    /// renders the scene for a render pass
    void renderScene(glow::camera::CameraBase* cam, RenderPass pass);

    // Input/Event handling
    bool onMouseButton(double x, double y, int button, int action, int mods, int clickCount) override;
    bool onMousePosition(double x, double y) override;
    bool onKey(int key, int scancode, int action, int mods) override;
    void onResize(int w, int h) override;
};
