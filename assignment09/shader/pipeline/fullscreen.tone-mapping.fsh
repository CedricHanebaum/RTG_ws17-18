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

    fColor = vec3(1, 0, 1);

    /// ============= STUDENT CODE END =============
}