/// ===============================================================================================
/// Task 1
/// Compute the terrain height at the given coordinate
///
/// Your job is to:
///     - compute some desert like terrain with dunes
///     - make sure the terrain height is 0 close to the pool
///
/// Notes:
///     - the method "cnoise(vec2 pos)" implements perlin noise, returns a float in [-1, 1] and is locally smooth
///     - you might need to scale the input parameter and the output parameter to get satisfying results
///     - for terrain generation, one typically uses multiple calls to cnoise with different magnitudes
///     - cnoise is somewhat expensive - don't call it too often
///     - an imagined sphere around the origin that contains at least the pool has radius "poolSize"
///     - the pool is centered at the origin
///
/// ============= STUDENT CODE BEGIN =============
float getTerrainHeight(vec2 pos)
{
    float distToOrigin = length(pos);

    // two layers perlin noise with different frequencies
    float height = 5*cnoise(0.02*pos.xy) + 0.2*distToOrigin*cnoise(0.001*pos.yx);

    // smooth interpolation between 0% and 100% from poolSize to poolSize * 2
    height *= smoothstep(0, poolSize, distToOrigin - poolSize);

    return height;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================