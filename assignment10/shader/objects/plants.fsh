#include "../common.glsl"

uniform sampler2D uTexPlants;

in vec3 vNormal;
in vec2 vTexCoord;
in vec2 vPosition;

out vec4 fColor;
out vec4 fMatA;
out vec2 fMatB;

void main()
{
    vec3 N = normalize(vNormal);

    vec4 color = texture(uTexPlants, vTexCoord);
    if (color.a < 0.5)
        discard;

    vec3 albedo = color.rgb;
    float ao = mix(0.1, 1.0, smoothstep(0.0, 1.0, vPosition.y));
    float metallic = 0.2;
    float roughness = 0.9;
    float translucency = 0.0;

    // write G-Buffer
    fColor = gBufferColor(albedo, ao);
    fMatA = gBufferMatA(N, metallic);
    fMatB = gBufferMatB(roughness, translucency);
}
