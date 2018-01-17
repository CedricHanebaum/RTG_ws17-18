
uniform float uRuntime;

uniform vec3 uCamPos;
uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uViewProj;
uniform float uRenderDistance;

// Helper

float distance2(vec3 a, vec3 b)
{
    vec3 d = a - b;
    return dot(d, d);
}

// G Buffer fill functions

vec4 gBufferColor(vec3 albedo, float ao)
{
    return vec4(albedo, ao);
}

vec4 gBufferMatA(vec3 worldNormal, float metallic)
{
    return vec4(worldNormal.xyz * 0.5 + 0.5, metallic);
}

vec2 gBufferMatB(float roughness, float translucency)
{
    return vec2(roughness, translucency);
}

// T Buffer fill functions

float oitWeight(float fragZ, float alpha)
{
    float d = 1 - fragZ;
    return alpha * max(1e-2, 3e3 * d * d * d);
}

vec4 tBufferAccumA(vec3 premultColor, float alpha, float w)
{
    return vec4(premultColor * w, alpha);
}

float tBufferAccumB(float alpha, float w)
{
    return alpha * w;
}

vec3 tBufferDistortion(vec2 offset, float blurriness)
{
    return vec3(offset, blurriness);
}
