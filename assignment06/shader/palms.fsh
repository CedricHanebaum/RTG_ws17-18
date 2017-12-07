#include "shading.glsl"

uniform bool uIsShadowPass;

uniform sampler2D uTexPalmColor;
uniform sampler2D uTexPalmNormal;
uniform sampler2D uTexPalmSpecular;

uniform vec3 uCamPos;

in vec2 vTexCoord;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vWorldPos;

out vec3 fColor;

void main()
{
    // Need tex-lookup for alpha discard here
    vec4 palmColor = texture(uTexPalmColor, vTexCoord);

    if (palmColor.a < 0.5)
        discard; // alpha-discard

    // early-out for shadow rendering
    if (uIsShadowPass)
    {
        fColor = vec3(1, 0, 1);
        return;
    }

    // calculate dirs
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    vec3 L = normalize(uLightDir);
    mat3 tbn = mat3(T, B, N);

    // material
    vec3 diffuse = palmColor.rgb;
    vec3 specular = vec3(smoothstep(0.2, 0.7, 1 - texture(uTexPalmSpecular, vTexCoord).r)) + float(vTexCoord.x > 0.5) * 0.1;
    specular *= 0.1;
    vec3 normalMap = texture(uTexPalmNormal, vTexCoord).rgb;
    float roughness = 0.3;
    float reflectivity = 0.2;
    float AO = 1.0;

    // normal mapping
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);

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
