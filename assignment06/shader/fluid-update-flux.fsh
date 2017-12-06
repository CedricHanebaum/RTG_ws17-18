#include "fluid.glsl"

out float fFluxX;
out float fFluxY;

void main()
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    // neutral values
    fFluxX = 0;
    fFluxY = 0;

    // vx is '(sizeX + 1) x sizeY'
    // .. and boundary condition is v = 0
    if (coords.y < uFluidSizeY && coords.x >= 1 && coords.x < uFluidSizeX)
        fFluxX = updateFluxX(coords);

    // vy is 'sizeX x (sizeY + 1)'
    // .. and boundary condition is v = 0
    if (coords.x < uFluidSizeX && coords.y >= 1 && coords.y < uFluidSizeY)
        fFluxY = updateFluxY(coords);
}
