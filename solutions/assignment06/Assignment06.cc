/// ===============================================================================================
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
/// ===============================================================================================



/// ===============================================================================================
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
// Palm tree textures
mTexPalmColor = Texture2D::createFromFile(texPath + "palm.color.png");
mTexPalmNormal = Texture2D::createFromFile(texPath + "palm.normal.png", ColorSpace::Linear); // <- note the linear space!
mTexPalmSpecular = Texture2D::createFromFile(texPath + "palm.specular.png");
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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
// No culling (especially for palm leaves)
GLOW_SCOPED(disable, GL_CULL_FACE);
auto shader = mShaderPalms->use();

shader.setUniform("uPalmPos", glm::vec3(-15, -0.3, -15));

shader.setTexture("uTexPalmColor", mTexPalmColor);
shader.setTexture("uTexPalmNormal", mTexPalmNormal);
shader.setTexture("uTexPalmSpecular", mTexPalmSpecular);

mMeshPalms->bind().draw();
/// ============= STUDENT CODE END =============
/// ===============================================================================================