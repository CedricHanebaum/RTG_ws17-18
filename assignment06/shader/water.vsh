#include "fluid.glsl"

uniform mat4 uView;
uniform mat4 uProj;

uniform float uPoolHeight;

in vec2 aPosition;

out vec2 vFluidCoords;
out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec3 vViewPos;
out vec4 vScreenPos;

void main()
{
    // get index into fluid grid
    int id = gl_InstanceID;
    float x = id % (uFluidSizeX - 1) + aPosition.x;
    float y = id / (uFluidSizeX - 1) + aPosition.y;
    vFluidCoords = vec2(x, y);

    // sample fluid
    float fheight = heightF(x, y);

    // calculate 3D world pos
    vec3 pos = vec3(
        x * uFluidResolution,
        fheight - uPoolHeight,
        y * uFluidResolution
    );

    pos.x *= (uFluidSizeX + 1.0) / uFluidSizeX;
    pos.z *= (uFluidSizeY + 1.0) / uFluidSizeY;

    // re-center pool
    pos -= vec3(uFluidSizeX, 0, uFluidSizeY) * uFluidResolution / 2.0;

    // calculate normal
    vec2 c = vFluidCoords.xy;
    c.x *= (uFluidSizeX - 1.0) / uFluidSizeX;
    c.y *= (uFluidSizeY - 1.0) / uFluidSizeY;
    float cenH = heightF(c.x + 0, c.y + 0) / uFluidResolution;
    float rigH = heightF(c.x + 1, c.y + 0) / uFluidResolution;
    float botH = heightF(c.x + 0, c.y + 1) / uFluidResolution;
    vec3 center = vec3(c.x,     cenH, c.y);
    vec3 right  = vec3(c.x + 1, rigH, c.y);
    vec3 bottom = vec3(c.x,     botH, c.y + 1);

    vNormal = normalize(cross(bottom - center, right - center));
    vTangent = right;

    // 3D transformation
    vWorldPos = pos;
    vViewPos = vec3(uView * vec4(vWorldPos, 1.0));
    gl_Position = uProj * uView * vec4(vWorldPos, 1.0);
    vScreenPos = gl_Position;
}
