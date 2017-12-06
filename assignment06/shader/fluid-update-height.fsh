#include "fluid.glsl"

uniform float uPoolHeight;

// Water interaction
uniform bool uWaterInteraction;
uniform vec2 uInteractionCoords;

out float fHeight;

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // neutral values
    fHeight = 0;

    // height is 'sizeX x sizeY'
    if (coords.x < uFluidSizeX && coords.y < uFluidSizeY)
        fHeight = updateHeight(coords);

    // Handle mouse/water interaction
    if(uWaterInteraction)
    {
        float dist = distance(uInteractionCoords, coords);
        if(dist < 3.0)
            fHeight += smoothstep(3, 0, dist);

    }

    // hard boundary
    if (coords.x == 0 ||
        coords.y == 0 ||
        coords.x == uFluidSizeX - 1 ||
        coords.y == uFluidSizeY - 1)
        fHeight = clamp(fHeight, 0.0, uPoolHeight);
}
