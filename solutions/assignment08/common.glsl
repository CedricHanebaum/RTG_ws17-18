/// Task 2.b
///
/// Your job is to:
///     - write down the correct T-Buffer values for Weighted OIT
///
/// Notes:
///     - the color is already pre-multiplied by alpha
///     - remember that revelage and accum.b are swapped
///
/// ============= STUDENT CODE BEGIN =============

vec4 tBufferAccumA(vec3 premultColor, float alpha, float w)
{
    return vec4(premultColor * w, alpha);
}

float tBufferAccumB(float alpha, float w)
{
    return alpha * w;
}

/// ============= STUDENT CODE END =============



/// Task 2.c
///
/// Your job is to:
///     - write offset and blurriness into T-Buffer
///
/// ============= STUDENT CODE BEGIN =============

vec3 tBufferDistortion(vec2 offset, float blurriness)
{
    return vec3(offset, blurriness);
}

/// ============= STUDENT CODE END =============
