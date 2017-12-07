#include "shading.glsl"

uniform bool uIsShadowPass;

uniform sampler2D uTexPoolColor;
uniform sampler2D uTexPoolNormal;
uniform sampler2D uTexPoolRoughness;
uniform sampler2D uTexPoolHeight;
uniform sampler2D uTexPoolAO;

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

    vec2 texCoord = vTexCoord / 6;

    // material
    vec3 albedo = texture(uTexPoolColor, texCoord).rgb;
    vec3 normalMap = texture(uTexPoolNormal, texCoord).rgb;
    float AO = texture(uTexPoolAO, texCoord).r;
    float metallic = 0.7f;
    float roughness = texture(uTexPoolRoughness, texCoord).r;
    float reflectivity = 0.35;

    // normal mapping
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
