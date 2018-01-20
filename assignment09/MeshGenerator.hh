#pragma once

#include <vector>

#include "TerrainMesh.hh"
#include "Block.hh"

/// Generates mesh data for a given array of blocks
/// Blocks contain 1 neighborhood
std::vector<TerrainMeshData> generateMesh(std::vector<Block> const& blocks, glm::ivec3 chunkPos);
