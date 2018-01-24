#pragma once

#include <glow/fwd.hh>

#include <glm/glm.hpp>

#include "Material.hh"
#include "Vertices.hh"

/// A terrain mesh for a (chunk, material, direction) combination
struct TerrainMesh
{
    /// The material used for this mesh
    SharedRenderMaterial mat = nullptr;

    /// Face Direction of this mesh
    glm::ivec3 dir;

    /// Bounding box
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;

    /// configured geometry
    glow::SharedVertexArray vaoFull;
    glow::SharedVertexArray vaoPosOnly;

    /// vertex data
    glow::SharedArrayBuffer abPositions;
    glow::SharedArrayBuffer abData;

    /// vegetation
    glow::SharedVertexArray vaoPlants;
    glow::SharedArrayBuffer abPlants;
};

/// A plant "seed"
struct Plant
{
    glm::vec3 position;
    glm::vec3 up;
    glm::vec3 left;
    int texId;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            {&Plant::position, "aPlantPosition"}, //
            {&Plant::up, "aPlantUp"},             //
            {&Plant::left, "aPlantLeft"},         //
            {&Plant::texId, "aPlantTexId"},       //
        };
    }
};

struct TerrainMeshData
{
    /// The material used for this mesh
    int8_t mat;

    /// Face Direction of this mesh
    glm::ivec3 dir;

    /// Bounding box
    glm::vec3 aabbMin;
    glm::vec3 aabbMax;

    /// Vertices
    std::vector<glm::vec3> vertexPositions;
    std::vector<TerrainVertex> vertexData;

    /// Vegetation
    std::vector<Plant> plants;
};
