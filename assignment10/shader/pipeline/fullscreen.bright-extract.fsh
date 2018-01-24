uniform sampler2DRect uTexture;

uniform float uToneMappingA;
uniform float uToneMappingGamma;
uniform float uBloomThreshold;

out vec3 fColor;

void main() 
{
    vec3 color = texelFetch(uTexture, ivec2(gl_FragCoord.xy)).rgb;

    // Bright extract depending on current tone mapping
    fColor = max(color * uToneMappingA - uBloomThreshold, vec3(0));
    fColor = fColor / (fColor + 1.0);
}
