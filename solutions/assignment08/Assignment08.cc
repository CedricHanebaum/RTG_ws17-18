/// Task 1.a
///
/// Your job is to:
///     - bind and clear the gbuffer (color only, not depth!)
///     - set the correct depthFunc
///     - enable writing to an sRGB framebuffer
///     - render the scene as opaque pass (renderScene(...))
///
/// Notes:
///     - do not render the scene if mPassOpaque is false
///     - getCamera().get() gives you a pointer to the current camera
///
/// ============= STUDENT CODE BEGIN =============

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

/// ============= STUDENT CODE END =============



/// Task 1.b
///
/// Your job is to:
///     - bind and clear the depth-pre buffer
///     - set the correct depthFunc state
///     - render the scene as depth-pre pass
///
/// Notes:
///     - do not render the scene if mPassDepthPre is false
///
/// ============= STUDENT CODE BEGIN =============

auto fb = mFramebufferDepthPre->bind();

// clear depth
glClear(GL_DEPTH_BUFFER_BIT);
GLOW_SCOPED(depthFunc, GL_LESS);

// render scene depth-pre
if (mPassDepthPre)
    renderScene(getCamera().get(), RenderPass::DepthPre);

/// ============= STUDENT CODE END =============



/// Task 1.c - fullscreen lighting pass
///
/// Your job is to:
///     - enable additive blending
///     - temporarily disable depth test and backface culling
///     - Perform a fullscreen lighting pass and render into the "ShadedOpaque" buffer
///
/// Notes:
///     - do not forget to clear the color of the buffer at first (not the depth!)
///     - do not forget to pick the right shader
///     - fullscreen passes are rendered using the mMeshQuad geometry (cf. renderOutput)
///
/// ============= STUDENT CODE BEGIN =============

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

/// ============= STUDENT CODE END =============



/// Task 1.c - point lights pass
///
/// Your job is to:
///     - render the mMeshLightSpheres geometry into the currently bound buffer
///
/// Notes:
///     - here we need culling and depth test again
///     - however, we must not WRITE the depth (see OpenGL depth mask)
///
/// ============= STUDENT CODE BEGIN =============

// we explicitly want culling
GLOW_SCOPED(enable, GL_CULL_FACE);

// depth TEST yes, depth WRITE no
GLOW_SCOPED(enable, GL_DEPTH_TEST);
GLOW_SCOPED(depthMask, GL_FALSE);

// draw lights
auto shader = mShaderPointLight->use();
mMeshLightSpheres->bind().draw();

/// ============= STUDENT CODE END =============



/// Task 2.b
/// Transparent Pass with Weighted, Blended OIT and Distortion
///
/// Your job is to:
///     - clear the T-Buffer (color only)
///     - set the correct render state
///     - write all transparent objects into the T-Buffer
///
/// Notes:
///     - we swapped revealage and accum.a so that we can use the same blend mode for all buffers
///       (RGB is additive, A is zero and 1-src.alpha)
///     - we want to render without culling and without writing depth (but with depth test)
///     - think about the correct clear color
///
/// ============= STUDENT CODE BEGIN =============

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

/// ============= STUDENT CODE END =============
