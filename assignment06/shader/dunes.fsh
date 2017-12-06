#include "shading.glsl"

uniform bool uIsShadowPass;

uniform sampler2D uTexSandColor;
uniform sampler2D uTexSandNormal;
uniform sampler2D uTexSandHeight;
uniform sampler2D uTexSandAO;

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


    // fix detail problems via hacks.
    vec2 texCoord = vTexCoord / 4;
    float wd = length(vWorldPos.xz);
    float normd = 1 + 20 * smoothstep(20, 0, sqrt(wd));
    float sdiv = (sqrt(wd) + normd) / normd;
    texCoord /= floor(sdiv);


    // calculate dirs
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    vec3 L = normalize(uLightDir);
    mat3 tbn = mat3(T, B, N);

    // material
    vec3 diffuse = texture(uTexSandColor, texCoord).rgb;
    vec3 normalMap = texture(uTexSandNormal, texCoord).rgb;
    float AO = texture(uTexSandAO, texCoord).r;
    vec3 specular = diffuse * 0.3;
    float roughness = 0.8;
    float reflectivity = 0.0;

    // normal mapping
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);

    // lighting    
    vec3 color = vec3(0.0);
    
    // ambient
    color += uAmbientLight * diffuse * AO;

    // diffuse
    color += uLightColor * shadingDiffuseOrenNayar(N, V, L, roughness, diffuse) * shadowing(vWorldPos, L);

    fColor = color;
}
