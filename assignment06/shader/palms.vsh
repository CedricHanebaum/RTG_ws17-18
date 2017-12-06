in vec3 aPosition;
in vec3 aNormal;
in vec3 aTangent;
in vec2 aTexCoord;

out vec2 vTexCoord;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vWorldPos;

uniform mat4 uView;
uniform mat4 uProj;
uniform vec3 uPalmPos;
uniform float uRuntime;

void main()
{
    vTexCoord = aTexCoord;
    vNormal = aNormal;
    vTangent = aTangent;
    vWorldPos = aPosition + uPalmPos;

    // "wind"
    {
        vec3 offset = vec3(
            sin(uRuntime * 0.5 + vWorldPos.x * 0.1),
            0,
            sin(uRuntime * 0.3 + 1.3 + vWorldPos.z * 0.1)
        ) * 0.6;
        
        vWorldPos += offset * smoothstep(0, 15, vWorldPos.y);
    }

    gl_Position = uProj * uView * vec4(vWorldPos, 1.0);
}
