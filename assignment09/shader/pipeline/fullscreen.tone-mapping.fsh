uniform sampler2DRect uTexHDR;
uniform sampler2DRect uTexBloomDownsampled;

uniform float uToneMappingA;
uniform float uToneMappingGamma;
uniform float uBloomStrength;

out vec3 fColor;

void main() 
{
    /// see Assignment09.cc
    /// 
    /// ============= STUDENT CODE BEGIN =============

    ivec2 coords = ivec2(gl_FragCoord.xy);
    fColor = uToneMappingA * pow(texelFetch(uTexHDR, coords).rgb, vec3(uToneMappingGamma));
    fColor += uBloomStrength * texture(uTexBloomDownsampled, coords * 0.5).rgb;

    /// ============= STUDENT CODE END =============
}