uniform sampler2DRect uTexture;

out vec3 fColor;

void main() 
{
    /// see Assignment09.cc
    /// 
    /// ============= STUDENT CODE BEGIN =============

    ivec2 coords = ivec2(gl_FragCoord.xy);
    fColor = texelFetch(uTexture, coords * 2).rgb;

    /// ============= STUDENT CODE END =============
}