#include "Chunk.hh"

#include <set>

#include <glm/ext.hpp>

#include <glow/common/log.hh>
#include <glow/common/profiling.hh>

#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

#include "Vertices.hh"
#include "World.hh"

using namespace glow;


Chunk::Chunk(glm::ivec3 chunkPos, int size, World *world) : chunkPos(chunkPos), size(size), world(world)
{
    mBlocks.resize(size * size * size);
}

SharedChunk Chunk::create(glm::ivec3 chunkPos, int size, World *world)
{
    if (chunkPos.x % size != 0 || chunkPos.y % size != 0 || chunkPos.z % size != 0)
        glow::error() << "Position must be a multiple of size!";

    // "new" because Chunk() is private
    return std::shared_ptr<Chunk>(new Chunk(chunkPos, size, world));
}

///
/// Create the meshes for this chunk. (one per material)
/// The return value is a map from material to mesh.
///
/// Your job is to:
///     - enhance the performance by (re-)creating meshes only if needed
///       (Dirty-flagging can be done using mIsDirty)
///     - create faces for all visible blocks
///     - adapt Vertices.hh (vertex type) and terrain.vsh (vertex shader)
///
///     - Advanced: do not create faces that are never visible from the outside
///                 - no face between two solids
///                 - no face between two translucent materials of the same type
///     - Advanced: compute some fake ambient occlusion for every vertex
///                 (also optimize the triangulation of the quads depending on the ao values)
///     - Advanced: Pack vertex data in 16 byte or less (e.g. a single (glm::)vec4)
///
///
/// Notes:
///     - pre-filled code is meant as a starting helper; you may change everything
///       you want inside the strips, e.g. implement helper functions (see Chunk.hh)
///     - for every material (except air) in a chunk, one mesh must be created
///     - it is up to you whether you create an indexed mesh or not
///     - local coordinates: 0..size-1            -> block query: block(localPos)
///     - global coordinates: chunkPos + localPos -> block query: queryBlock(globalPos)
///     - read Chunk.hh for helpers and explanations
///     - for AO see screenshots, "Fake Ambient Occlusion.jpg", and "Fake AO - Triangulation.jpg"
///     - don't forget that some faces need information from neighboring chunks (global queries)
///
/// ============= STUDENT CODE BEGIN =============
std::map<int, SharedVertexArray> Chunk::queryMeshes()
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    if(!mIsDirty) return mMeshes;

    // clear list of cached meshes
    mMeshes.clear();
    std::set<int> built; // track already built materials

    // ensure that each material is accounted for
    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                glm::ivec3 p = { x, y, z };
                auto const &b = block(p); // get block

                // if block material is not air and not already built
                if (!b.isAir() && !built.count(b.mat))
                {
                    // mark as built
                    built.insert(b.mat);

                    // create VAO
                    auto vao = buildMeshFor(b.mat);
                    if (vao) // might be nullptr if fully surrounded
                        mMeshes[b.mat] = vao;
                }
            }

    mIsDirty = false;

    glow::info() << "Rebuilding mesh for " << chunkPos;
    return mMeshes;
}

SharedVertexArray Chunk::buildMeshFor(int mat) const
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    // assemble data
    std::vector<TerrainVertex> vertices;

    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                glm::ivec3 p = { x, y, z }; // local position
                auto gp = chunkPos + p;     // global position
                auto const &blk = block(p);

                if (blk.mat != mat)
                    continue; // consider only current material

                // go over all 6 directions
                for (auto s : { -1, 1 })
                    for (auto dir : { 0, 1, 2 })
                    {
                        // face normal
                        auto n = s * glm::ivec3(dir == 0, dir == 1, dir == 2);

                        // TODO!
                    }
            }

    if (vertices.empty())
        return nullptr; // no visible faces

    auto ab = ArrayBuffer::create(TerrainVertex::attributes());
    ab->bind().setData(vertices);

    glow::info() << "Created " << vertices.size() << " verts for mat " << mat << " in chunk " << chunkPos;
    return VertexArray::create(ab);
}

float Chunk::aoAt(glm::ivec3 pos, glm::ivec3 dx, glm::ivec3 dy) const
{
    return 1.0f; // TODO
}
/// ============= STUDENT CODE END =============

void Chunk::markDirty()
{
    mIsDirty = true;

    mMeshes.clear();
}

const Block &Chunk::queryBlock(glm::ivec3 worldPos) const
{
    if (contains(worldPos))
        return block(worldPos - chunkPos);
    else
        return world->queryBlock(worldPos);
}
