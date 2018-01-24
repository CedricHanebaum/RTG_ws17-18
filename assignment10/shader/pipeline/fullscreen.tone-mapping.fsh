uniform sampler2DRect uTexHDR;
uniform sampler2DRect uTexBloomDownsampled;

uniform float uToneMappingA;
uniform float uToneMappingGamma;
uniform float uBloomStrength;

out vec3 fColor;

void main() 
{
    vec3 hdrColor = texelFetch(uTexHDR, ivec2(gl_FragCoord.xy)).rgb;
    vec3 bloom = texture(uTexBloomDownsampled, gl_FragCoord.xy / 2).rgb; // correct? TODO: slight blur

    // tone mapping
    vec3 color = uToneMappingA * pow(hdrColor, vec3(uToneMappingGamma));

    // bloom
    color += bloom * uBloomStrength;

    fColor = color;
}
