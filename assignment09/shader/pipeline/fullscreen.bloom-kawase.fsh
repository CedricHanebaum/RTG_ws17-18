uniform sampler2DRect uTexture;

uniform float uDistance;

out vec3 fColor;

void main() 
{
    /// see Assignment09.cc
    /// 
    /// ============= STUDENT CODE BEGIN =============

    ivec2 coords = ivec2(gl_FragCoord.xy);
    int d = int(uDistance);

    // bottom right
    vec3 s1 = texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, 0)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(1, 0)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, 1)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(1, 1)).rgb;

    // bottom left
    vec3 s2 = texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(-1, 0)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, 0)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(1, -1)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(1, 0)).rgb;

    // top left
    vec3 s3 = texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(-1, -1)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, -1)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(-1, 0)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, 0)).rgb;

    // top right
    vec3 s4 = texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, -1)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(1, -1)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(0, 0)).rgb
            + texelFetch(uTexture, coords + d * ivec2(1, 1) + ivec2(1, 0)).rgb;

    fColor = (s1 + s2 + s3 + s4) / 16;

    /// ============= STUDENT CODE END =============
}