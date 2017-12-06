in vec2 vPosition;

uniform samplerCube uTexCubeMap;

uniform mat4 uInvProj;
uniform mat4 uInvView;
uniform vec3 uLightDir;

out vec3 fColor;

void main() 
{
    vec4 near = uInvProj * vec4(vPosition * 2 - 1, 0, 1);
    vec4 far = uInvProj * vec4(vPosition * 2 - 1, 0.5, 1);

    near /= near.w;
    far /= far.w;

    near = uInvView * near;
    far = uInvView * far;

    fColor = texture(uTexCubeMap, (far - near).xyz).rgb;

    vec3 sunColor = vec3(1,0.7,0.3);
    float dotVL = dot(uLightDir, normalize(far - near).xyz);
    fColor += smoothstep(0.993, 0.999, dotVL) * sunColor;

    fColor = pow(fColor, vec3(1 / 2.2));
}
