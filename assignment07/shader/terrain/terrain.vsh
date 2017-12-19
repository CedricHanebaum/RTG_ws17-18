out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vTangent;
out vec2 vTexCoord;
out vec3 vViewPos;
out vec4 vScreenPos;
out float vAO;

uniform mat4 uProj;
uniform mat4 uView;
uniform float uTextureScale;

///
/// Vertex shader code for all terrain materials
///
/// Your job is to:
///     - define vertex attribute(s) for the data you need
///     - compute tangent and texture coordinate
///     - write to all defined out-variables (vWorldPos, ...)
///
///     - Advanced: unpack position, ao value and normal from your optimized vertex format
///
/// Notes:
///     - don't forget to divide texture coords by uTextureScale
///
/// ============= STUDENT CODE BEGIN =============

in ivec3 aPosition;
in int aData;
in float aAO;

int getDir(int data) {
    return data & 3;
}

int getSide(int data) {
    return ((data >> 2) & 1) == 0 ? -1 : 1;
}

int getLocation(int data) {
    return (data >> 3) & 3;
}

vec3 getNormal(int side, int dir) {
    return side * vec3(dir == 0, dir == 1, dir == 2);
}

vec2 getTexCoord(int loc) {
    return vec2(loc == 0 || loc == 1 ? 0 : 1, loc == 0 || loc == 2 ? 0 : 1);
}

void main()
{
    vNormal = getNormal(getSide(aData), getDir(aData));
    vTangent = vec3(vNormal.y, vNormal.z, vNormal.x);
    vTexCoord = getTexCoord(getLocation(aData)); // don't forget uTextureScale
    vAO = 1.0f;

    vWorldPos = aPosition;
    vViewPos = vec3(uView * vec4(aPosition, 1.0));
    vScreenPos = uProj * vec4(vViewPos, 1.0);

    gl_Position = vScreenPos;
}

/// ============= STUDENT CODE END =============
