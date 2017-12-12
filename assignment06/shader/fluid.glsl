uniform sampler2DRect uFluidHeight;
uniform sampler2DRect uFluidFluxX;
uniform sampler2DRect uFluidFluxY;

uniform int uFluidSizeX;
uniform int uFluidSizeY;
uniform float uFluidResolution;
uniform float uFluidDrag;
uniform float uDeltaT;
uniform float uGravity;
uniform float uViscosityTau;
uniform float uLinearDragTau;

// Query flux in x direction
float fluxX(int cx, int cy)
{
    return texelFetch(uFluidFluxX, ivec2(cx, cy)).x;
}
// Query flux in y direction
float fluxY(int cx, int cy)
{
    return texelFetch(uFluidFluxY, ivec2(cx, cy)).x;
}
// Query flux in x direction (linearly interpolated)
float fluxXF(float cx, float cy)
{
    return texture(uFluidFluxX, vec2(cx, cy)).x;
}
// Query flux in y direction (linearly interpolated)
float fluxYF(float cx, float cy)
{
    return texture(uFluidFluxY, vec2(cx, cy)).x;
}
// Query height in cell x,y
float height(int cx, int cy)
{
    return texelFetch(uFluidHeight, ivec2(cx, cy)).x;
}
// Query height (linearly interpolated)
float heightF(float cx, float cy)
{
    return texture(uFluidHeight, vec2(cx, cy)).x;
}


/// Optional: Do the divergence computation for task 3.a here
/// Your task 3.a should use div(...) while divF(...) is used
/// for the rendering mode "Divergence".
/// ============= STUDENT CODE BEGIN =============
float div(int cx, int cy)
{
    return fluxX(cx + 1, cy) - fluxX(cx, cy) +
        fluxY(cx, cy + 1) - fluxY(cx, cy);
}
// Query divergence (linearly interpolated)
float divF(float cx, float cy)
{
    return fluxXF(cx + 1, cy) - fluxXF(cx, cy) +
        fluxYF(cx, cy + 1) - fluxYF(cx, cy);
}
/// ============= STUDENT CODE END =============

/// Task 3.a
/// Update the water height at the given coordinats
///
/// Your job is to:
///     - determine the divergence in the current grid cell
///     - compute the new water height value
///     - account for fluid viscosity, depending on uViscosityTau
///     - return the new height value
///
/// Notes:
///     - query the height at integer coordinates x and y by calling height(x, y)
///     - query the flux at integer coordinates x and y by calling fluxX(x, y) and fluxY(x, y)
///     - the side length of one fluid cell is uFluidResolution
///     - you should make sure that the water height never reaches 0 (or less)
///       to circumvent that, you can just set it some epsilon in that case
///     - do not query height out of [0..uFluidSizeX-1]x[0..uFluidSizeY-1]
///     - time step is in uDeltaT
///     - if you want, you can move the computation of the divergence to the
///       methods divF(float cx, float cy) and div(int cx, int cy)
///       div(...) is what you should use here while divF(...) is used for the div. render mode
///
///     - see fluid-equations.jpg for the required math
///
/// ============= STUDENT CODE BEGIN =============
float updateHeight(ivec2 coords)
{
    float eps = 0.001;

    float area = uFluidResolution * uFluidResolution;
    float delta = -(1/area) * div(coords.x, coords.y);
    float height = height(coords.x, coords.y) + delta * uDeltaT;
    return max(mix(height - delta * uDeltaT, height, pow(0.5, uDeltaT / uViscosityTau)), eps);
}
/// ============= STUDENT CODE END =============


/// Task 3.b
/// Update the flux in x and y directions
///
/// Your job is to:
///     - compute the flux in x and y direction (two methods)
///     - compute the flux update (use 9.81 for the gravity constant g)
///     - account for linear drag using uLinearDragTau
///     - return the new flux value
///
/// Notes:
///     - query the height at integer coordinates x and y by calling height(x, y)
///     - query the flux at integer coordinates x and y by calling fluxX(x, y) and fluxY(x, y)
///     - For more stability, clamp new flux value e.g. to [-5, 5]
///     - time step is in uDeltaT
///
///     - see fluid-equations.jpg for the required math
///
/// ============= STUDENT CODE BEGIN =============
float updateFluxX(ivec2 coords)
{
    float delta = -9.81 *
        (height(coords.x, coords.y) - height(coords.x - 1, coords.y)) *
        ((height(coords.x, coords.y) + height(coords.x - 1, coords.y)) / 2);
    float fX = fluxX(coords.x, coords.y) + delta * uDeltaT;
    return clamp(mix(0, fX, pow(0.5, uDeltaT / uLinearDragTau)), -5, 5);
}

float updateFluxY(ivec2 coords)
{
    float delta = -9.81 *
        (height(coords.x, coords.y) - height(coords.x, coords.y - 1)) *
        ((height(coords.x, coords.y) + height(coords.x, coords.y - 1)) / 2);
    float fY = fluxY(coords.x, coords.y) + delta * uDeltaT;
    return clamp(mix(0, fY, pow(0.5, uDeltaT / uLinearDragTau)), -5, 5);
}
/// ============= STUDENT CODE END =============
