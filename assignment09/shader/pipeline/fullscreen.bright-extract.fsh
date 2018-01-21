uniform sampler2DRect uTexture;

uniform float uToneMappingA;
uniform float uToneMappingGamma;
uniform float uBloomThreshold;

out vec3 fColor;

void main() 
{
    /// see Assignment09.cc
    /// 
    /// ============= STUDENT CODE BEGIN =============

    ivec2 coords = ivec2(gl_FragCoord.xy);
    vec3 c = texelFetch(uTexture, coords).rgb;

    vec3 c_ = max(c * uToneMappingA - uBloomThreshold, vec3(0, 0, 0));
    fColor = c_ / (c_ + 1);

    /// ============= STUDENT CODE END =============
}