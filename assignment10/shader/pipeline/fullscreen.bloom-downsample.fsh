uniform sampler2DRect uTexture;

out vec3 fColor;

void main() 
{
    fColor = texture(uTexture, gl_FragCoord.xy * 2).rgb;
}
