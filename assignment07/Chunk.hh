#pragma once

#include <map>
#include <vector>

#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include "Block.hh"
#include "Vertices.hh"

GLOW_SHARED(class, Chunk);
class World;
class Chunk
{
public: // properties
    /// chunk start position in [m]
    /// chunk goes from chunkPos .. chunkPos + size-1
    /// a block at (x,y,z) goes from (x,y,z)..(x+1,y+1,z+1)
    const glm::ivec3 chunkPos;

    /// chunk size in x,y,z dir (i.e. number of blocks per side)
    /// each block is 1m x 1m x 1m
    const int size;

    /// Backreference to the world
    World* const world = nullptr;

    /// returns true iff mesh is outdated
    bool isDirty() const { return mIsDirty; }

    /// returns the world space center of this chunk
    glm::vec3 chunkCenter() const { return glm::vec3(chunkPos) + size / 2.0f; }

private: // private members
    /// List of blocks
    /// Use block(...) functions!
    std::vector<Block> mBlocks;

    /// This chunk's configured meshes
    /// Map is from material ID to vertex array
    std::map<int, glow::SharedVertexArray> mMeshes;

    /// if true, the list of blocks has changed and the mesh might be invalid
    bool mIsDirty = true;

private: // ctor
    Chunk(glm::ivec3 chunkPos, int size, World* world);

public: // create
    static SharedChunk create(glm::ivec3 chunkPos, int size, World* world);

public: // gfx
    /// returns an up-to-date version of the current meshes
    /// (might lazily rebuild the mesh)
    /// there is one mesh for each material
    std::map<int, glow::SharedVertexArray> queryMeshes();

private: // gfx helper
/// All Tasks
///
/// You can use this space for declarations of helper functions
///
/// ============= STUDENT CODE BEGIN =============

    /// Builds the mesh for a given material
    glow::SharedVertexArray buildMeshFor(int mat) const;
    /// Returns the ambient occlusion at a given position
    int aoAt(glm::ivec3 pos, glm::ivec3 dx, glm::ivec3 dy) const;
    /// Generates the vertices for the block at the given position and adds them to the vertex list.
    void generateBlock(std::vector<TerrainVertex>& vertices, glm::ivec3 p, glm::ivec3 gp) const;
    /// Generates the vertices of a single face
    void generateFace(std::vector<TerrainVertex>& vertices, glm::ivec3 gp, int dir, int s) const;
    /// Checks if the given Face is visible, based on its neighbouring block
    bool isVisible(glm::ivec3 gp, int dir, int s) const;

/// ============= STUDENT CODE END =============

public: // modification funcs
    /// Marks this chunk as "dirty" (triggers rebuild of mesh)
    void markDirty();

public: // accessor functions
    /// relative coordinates 0..size-1
    /// do not call outside that range
    Block& block(glm::ivec3 relPos) { return mBlocks[(relPos.z * size + relPos.y) * size + relPos.x]; }
    Block const& block(glm::ivec3 relPos) const { return mBlocks[(relPos.z * size + relPos.y) * size + relPos.x]; }

    /// returns true iff these global coordinates are contained in this block
    bool contains(glm::ivec3 p) const
    {
        return chunkPos.x <= p.x && p.x < chunkPos.x + size && //
               chunkPos.y <= p.y && p.y < chunkPos.y + size && //
               chunkPos.z <= p.z && p.z < chunkPos.z + size;
    }

    /// queries a block in global coordinates
    /// will first search locally and otherwise consult world
    Block const& queryBlock(glm::ivec3 worldPos) const;
};
