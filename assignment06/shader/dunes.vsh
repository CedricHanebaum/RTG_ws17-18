in vec2 aPosition;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vWorldPos;

uniform mat4 uView;
uniform mat4 uProj;

uniform int uCountAngles;
uniform int uCountRadii;
uniform int uFluidSizeX;
uniform int uFluidSizeY;
uniform float uFluidResolution;
uniform float uPoolBorder;

#include "noise2D.glsl"

float rW = uPoolBorder * 0.5 + uFluidSizeX * uFluidResolution / 2.0;
float rH = uPoolBorder * 0.5 + uFluidSizeY * uFluidResolution / 2.0;
float poolSize = 2 * max(rW, rH);

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
float lnEps = -13.81551055796427410410794872810618524560660893177263;
float e = 2.7182818284590452353602874713526624977572;

float taper(float dist, float taperMin, float taperMax) {
    float a = lnEps / (2 * (taperMin - taperMax));
    float x = dist - taperMax;
    return pow(e, a * (x - abs(x)));
}

float getTerrainHeight(vec2 pos)
{
    float taperMax = 2.5 * poolSize;

    float noiseL = 8 * cnoise(pos * 20);
    float noiseS = 0.2 * cnoise(pos * 0.5); noiseS = 0;

    return taper(length(pos), poolSize, taperMax) * ((noiseL + noiseS) / 2);
}
/// ============= STUDENT CODE END =============

vec3 applyTerrainHeight(vec3 pos)
{
    return vec3(pos.x, getTerrainHeight(pos.xz) - 0.001, pos.z);
}

void main() 
{
    int id = gl_InstanceID;
    int x = id % uCountAngles;
    int y = id / uCountAngles;

    float angle = 2 * 3.14159265359 * (x + aPosition.x) / uCountAngles;
    float r = (y + aPosition.y) / uCountRadii;

    vec3 pos = vec3(
        sin(angle),
        0,
        cos(angle)
    );

    vec3 pV = pos * rW / abs(pos.x + 0.001);
    vec3 pH = pos * rH / abs(pos.z + 0.001);
    vec3 pR = dot(pV, pV) < dot(pH, pH) ? pV : pH;

    pos = mix(pR, pos * 3000, r*r*r);
    vWorldPos = applyTerrainHeight(pos);

    // make additional noise samples so that we can compute normals
    vec3 posR = applyTerrainHeight(vWorldPos + vec3(1,0,0));
    vec3 posT = applyTerrainHeight(vWorldPos + vec3(0,0,1));
    vNormal = (cross(posT - vWorldPos, posR - vWorldPos));
    vTangent = cross(vNormal, vec3(0,0,1));
    vTexCoord = vec2(vWorldPos.xz);

    gl_Position = uProj * uView * vec4(vWorldPos, 1.0);
}
