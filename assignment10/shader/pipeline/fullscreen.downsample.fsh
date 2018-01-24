#include "../common.glsl"
#include "../shading.glsl"

in vec2 vPosition;

uniform sampler2DRect uTexShadedOpaque;

out vec4 fColor;
out vec4 fMatA;
out vec4 fMatB;

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy) * 2;

    ivec2 coords2 = coords - ivec2(1, 0);
    ivec2 coords3 = coords - ivec2(1, 1);
    ivec2 coords4 = coords - ivec2(0, 1);

    float depth = texelFetch(uTexOpaqueDepth, coords).x;
    float depth2 = texelFetch(uTexOpaqueDepth, coords2).x;
    float depth3 = texelFetch(uTexOpaqueDepth, coords3).x;
    float depth4 = texelFetch(uTexOpaqueDepth, coords4).x;

    ivec2 newCoords = coords;
    if(depth2 < depth)
    {
        depth = depth2;
        newCoords = coords2;
    }
    if(depth3 < depth)
    {
        depth = depth3;
        newCoords = coords3;
    }
    if(depth4 < depth)
    {
        depth = depth4;
        newCoords = coords4;
    }

    fColor = texelFetch(uTexShadedOpaque, newCoords);
    //fColor = texture(uTexShadedOpaque, gl_FragCoord.xy*2);
    fMatA = texelFetch(uTexGBufferMatA, newCoords);
    fMatB = texelFetch(uTexGBufferMatB, newCoords);
    gl_FragDepth = depth;
}
