/// ===============================================================================================
/// Optional: Do the divergence computation for task 3.a here
/// Your task 3.a should use div(...) while divF(...) is used
/// for the rendering mode "Divergence".
/// ============= STUDENT CODE BEGIN =============
// Query divergence
float div(int cx, int cy)
{
    float vx_dx = fluxX(cx + 1, cy) - fluxX(cx, cy);
    float vy_dy = fluxY(cx, cy + 1) - fluxY(cx, cy);

    // Divergence is sum of flux change in x and y direction
    return vx_dx + vy_dy;
}

// Query divergence (linearly interpolated)
float divF(float cx, float cy)
{
    float vx_dx = fluxXF(cx + 1, cy) - fluxXF(cx, cy);
    float vy_dy = fluxYF(cx, cy + 1) - fluxYF(cx, cy);

    // Divergence is sum of flux change in x and y direction
    return vx_dx + vy_dy;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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
// ============= STUDENT CODE BEGIN =============
float updateHeight(ivec2 coords)
{
    // [m^3 / s]
    float sum_hv = fluxX(coords.x + 1, coords.y)
                 - fluxX(coords.x, coords.y)
                 + fluxY(coords.x, coords.y + 1)
                 - fluxY(coords.x, coords.y);

    // calculate height gradient
    float dh_dt = -sum_hv / (uFluidResolution * uFluidResolution);

    // apply gradient
    float fHeight = height(coords.x, coords.y) + dh_dt * uDeltaT;

    // make sure that height > 0
    fHeight = max(0.0001, fHeight);

    // Viscosity via laplacian smoothing
    float h1 = height(max(0, coords.x - 1), max(0, coords.y - 1));
    float h2 = height(max(0, coords.x - 1), min(uFluidSizeY - 1, coords.y + 1));
    float h3 = height(min(uFluidSizeX - 1, coords.x + 1), max(0, coords.y - 1));
    float h4 = height(min(uFluidSizeX - 1, coords.x + 1), min(uFluidSizeY - 1, coords.y + 1));

    float currentHeight = fHeight;
    float targetHeight = (h1+h2+h3+h4) / 4.0;

    // make smoothing depend on time step
    return mix(targetHeight, currentHeight, pow(0.5, uDeltaT / uViscosityTau));
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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

float maxFlux = 5; // flux limit for stability

float updateFluxX(ivec2 coords)
{
    // query adjacent height
    // note: indices are -1 and -0 (due to staggered grid)
    float h0 = height(coords.x - 1, coords.y);
    float h1 = height(coords.x - 0, coords.y);

    float hDiff = h1 - h0;
    float currHV = fluxX(coords.x, coords.y);

    // calculate gradient
    float dhv_dt = -9.81 * (h0 + h1) / 2 * hDiff;

    // apply gradient
    float fFluxX = currHV + dhv_dt * uDeltaT;

    // clamp flux for stability
    fFluxX = clamp(fFluxX, -maxFlux, maxFlux);

    // linear damping
    fFluxX *= pow(0.5, uDeltaT / uLinearDragTau);

    return fFluxX;
}

float updateFluxY(ivec2 coords)
{
    // see updateFluxX

    float h0 = height(coords.x, coords.y - 1);
    float h1 = height(coords.x, coords.y - 0);

    float hDiff = h1 - h0;


    float currHV = fluxY(coords.x, coords.y);

    float dhv_dt = -9.81 * (h0 + h1) / 2 * hDiff;


    float fFluxY = currHV + dhv_dt * uDeltaT;
    fFluxY = clamp(fFluxY, -maxFlux, maxFlux);
    fFluxY *= pow(0.5, uDeltaT / 5.0);

    return fFluxY;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================