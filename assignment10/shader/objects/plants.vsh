#include "../noise2D.glsl"

in vec2 aPosition;

in vec3 aPlantPosition;
in vec3 aPlantUp;
in vec3 aPlantLeft;
in int aPlantTexId;

out vec2 vTexCoord;
out vec3 vNormal;
out vec2 vPosition;

uniform mat4 uProj;
uniform mat4 uView;
uniform mat4 uViewProj;

uniform float uRuntime;

void main()
{
    vec2 windDir = normalize(vec2(.5, 1));
    vec2 wind = vec2(0, 0);
    vec3 pos = vec3(0);
    vec3 normal = vec3(0);

    /// The billboards are rendered using instancing
    /// That means aPosition is a quad (from 0,0 .. 1,1)
    /// aPlantXYZ contains per-instance plant data (see C++ side)
    /// 
    /// Your task is to:
    ///     - calculate the vertex position `pos`
    ///     - calculate the vertex normal `normal`
    ///     - calculate a wind offset vector `wind`
    /// 
    /// Notes:
    ///     - cnoise(vec2) returns a perlin noise value (-1..1)
    ///     - wind should vary with time AND space
    ///     - current time (in seconds) is in uRuntime
    ///     - aPosition.y goes from 0 (bottom) to 1 (top)
    ///     - the meaning of aPlantXYZ is given in the C++ task
    /// 
    /// ============= STUDENT CODE BEGIN =============

    // Instancing Example: 
    // .. aPosition changes per vertex
    pos = vec3(aPosition.x, aPosition.y, 0);
    // .. aPlantPosition changes per instance
    pos += aPlantPosition;

    /// ============= STUDENT CODE END =============

    // wind only applies to the top
    pos.xz += wind * aPosition.y;

    int id = aPlantTexId + 3;
    vTexCoord = vec2(aPosition.x, 1 - aPosition.y) * 0.5;    
    vTexCoord.x += float(id % 2 == 0) * 0.5;
    vTexCoord.y += float(id / 2 % 2 == 0) * 0.5;
    vPosition = aPosition;
    vNormal = normal;

    gl_Position = uViewProj * vec4(pos, 1.0);
}
