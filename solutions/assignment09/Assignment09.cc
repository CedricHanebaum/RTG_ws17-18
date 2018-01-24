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

GLOW_SCOPED(disable, GL_DEPTH_TEST);
GLOW_SCOPED(disable, GL_CULL_FACE);

// extract bright areas
{
    auto fb = mFramebufferBrightExtract->bind();
    auto shader = mShaderBrightExtract->use();
    shader.setTexture("uTexture", mTexHDRColor);

    shader.setUniform("uToneMappingA", mToneMappingA);
    shader.setUniform("uToneMappingGamma", mToneMappingGamma);
    shader.setUniform("uBloomThreshold", mBloomThreshold);

    mMeshQuad->bind().draw();
}

// downsample
{
    auto fb = mFramebufferBloomToA->bind();
    auto shader = mShaderBloomDownsample->use();
    shader.setTexture("uTexture", mTexBrightExtract);

    mMeshQuad->bind().draw();
}

// kawase blur
// - input: Bloom A
// - output: Bloom A (always, via swap)
{
    auto shader = mShaderBloomKawase->use();
    for (float dis : {0, 1, 2, 2, 3})
    {
        auto fb = mFramebufferBloomToB->bind();
        shader.setTexture("uTexture", mTexBloomDownsampledB);
        shader.setUniform("uDistance", dis);

        mMeshQuad->bind().draw();

        swapBloomTargets();
    }
}

// tone mapping
{
    auto fb = mFramebufferLDRColor->bind();
    auto shader = mShaderToneMapping->use();
    shader.setTexture("uTexHDR", mTexHDRColor);
    shader.setTexture("uTexBloomDownsampled", mTexBloomDownsampledA);

    shader.setUniform("uToneMappingA", mToneMappingA);
    shader.setUniform("uToneMappingGamma", mToneMappingGamma);
    shader.setUniform("uBloomStrength", mBloom ? mBloomStrength : 0.0f);

    mMeshQuad->bind().draw();
}

/// ============= STUDENT CODE END =============