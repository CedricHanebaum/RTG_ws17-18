#include "shading.glsl"

uniform bool uIsShadowPass;

uniform sampler2D uTexLadderColor;
uniform sampler2D uTexLadderNormal;
uniform sampler2D uTexLadderRoughness;
uniform sampler2D uTexLadderAO;

uniform vec3 uCamPos;

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vWorldPos;

out vec3 fColor;

void main()
{
    // early-out for shadow rendering
    if (uIsShadowPass)
    {
        fColor = vec3(1, 0, 1);
        return;
    }

    // calculate dirs
    vec3 N = normalize(vNormal);
    vec3 T = normalize(vTangent);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 B = normalize(cross(T, N));
    vec3 L = normalize(uLightDir);
    mat3 tbn = mat3(T, B, N);

    // material
    vec3 albedo = texture(uTexLadderColor, vTexCoord).rgb;
    vec3 normalMap = texture(uTexLadderNormal, vTexCoord).rgb;
    float AO = texture(uTexLadderAO, vTexCoord).r;
    float metallic = 1.0f;
    float roughness = texture(uTexLadderRoughness, vTexCoord).r;
    float reflectivity = 0.35;

    // normal mapping
    normalMap.z *= 0.4;
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);
    
    // diffuse/specular
    vec3 diffuse = albedo * (1 - metallic); // metals have no diffuse
    vec3 specular = mix(vec3(0.04), albedo, metallic); // fixed spec for non-metals

    // lighting
    fColor = shadingComplete(
        vWorldPos,
        N, V, L,
        AO,
        roughness,
        diffuse,
        specular,
        reflectivity
    );
}
