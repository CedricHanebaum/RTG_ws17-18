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
    vec3 hdrColor = texelFetch(uTexHDR, ivec2(gl_FragCoord.xy)).rgb;
    vec3 bloom = texture(uTexBloomDownsampled, gl_FragCoord.xy / 2).rgb;

    // tone mapping
    vec3 color = uToneMappingA * pow(hdrColor, vec3(uToneMappingGamma));

    // bloom
    color += bloom * uBloomStrength;

    fColor = color;
    /// ============= STUDENT CODE END =============
}