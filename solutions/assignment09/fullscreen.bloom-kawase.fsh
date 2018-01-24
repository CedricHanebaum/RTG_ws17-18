uniform sampler2DRect uTexture;

uniform float uDistance;

out vec3 fColor;

void main() 
{
    /// see Assignment09.cc
    /// 
    /// ============= STUDENT CODE BEGIN =============
    vec3 color = vec3(0);
    color += texture(uTexture, gl_FragCoord.xy + (uDistance + 0.5) * vec2(+1, +1)).rgb;
    color += texture(uTexture, gl_FragCoord.xy + (uDistance + 0.5) * vec2(+1, -1)).rgb;
    color += texture(uTexture, gl_FragCoord.xy + (uDistance + 0.5) * vec2(-1, +1)).rgb;
    color += texture(uTexture, gl_FragCoord.xy + (uDistance + 0.5) * vec2(-1, -1)).rgb;
    fColor = color / 4.0;
    /// ============= STUDENT CODE END =============
}