#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <glow/objects/ArrayBufferAttribute.hh>

///
/// The vertex type used for all terrain meshes
///
/// Your job is to:
///     - create/modify the vertex type to fit your needs
///
///     - Advanced: Pack everything you need in 16 byte or less (e.g. a single (glm::)vec4)
///
/// Notes:
///     - do not upload tangents or texture coords!
///       (those can -and have to- be computed in the shader)
///
/// Grading:
///     3p if <= 16 byte per vertex
///     2p if <= 28 byte per vertex
///     1p otherwise
///
///     (~4.9 byte is theoretical minimum but <= 16 is fine for full points)
///
/// ============= STUDENT CODE BEGIN =============

struct TerrainVertex
{
    glm::ivec3 pos;
    // dir: xxxx xx11
    // s  : xxxx x1xx
    // loc: xxx1 1xxx
    // ao : x11x xxxx
    int data;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            { &TerrainVertex::pos, "aPosition" },  //
            { &TerrainVertex::data, "aData"},
        };
    }
};

/// ============= STUDENT CODE END =============
