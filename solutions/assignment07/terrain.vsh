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
in vec4 aPosition;

void main()
{
    vec3 pos = aPosition.xyz;
    float ao = fract(aPosition.w) * 2;
    int pdir = int(aPosition.w);
    int s = pdir / 3 * 2 - 1;
    int dir = pdir % 3;

    vec3 N = vec3(float(dir == 0), float(dir == 1), float(dir == 2)) * float(s);

    // derive tangent
    vec3 T = normalize(cross(abs(N.x) > abs(N.y) + 0.1 ? vec3(0,1,0) : vec3(1,0,0), N));
    vec3 B = normalize(cross(T, N));

    // derive UV
    vTexCoord = vec2(
        dot(pos, T),
        dot(pos, B)
    ) / uTextureScale;
    
    vAO = ao;
    vNormal = N;
    vTangent = T;

    vWorldPos = pos;
    vViewPos = vec3(uView * vec4(pos, 1.0));
    vScreenPos = uProj * vec4(vViewPos, 1.0);

    gl_Position = vScreenPos;
}
/// ============= STUDENT CODE END =============