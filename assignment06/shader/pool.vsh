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

void main() 
{
    vTexCoord = aTexCoord;
    vNormal = aNormal;
    vTangent = aTangent;
    vWorldPos = aPosition;
    gl_Position = uProj * uView * vec4(aPosition, 1.0);
}
