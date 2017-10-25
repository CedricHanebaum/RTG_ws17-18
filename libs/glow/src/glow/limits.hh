#pragma once

namespace glow
{
/// OpenGL limits
namespace limits
{
/// number of texture units
extern int maxCombinedTextureImageUnits;
/// max value for anisotropic filtering
extern float maxAnisotropy;

/// updates all limits
/// Is done in glowInit as well
void update();
}
}
