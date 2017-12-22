#pragma once

#include <vector>

#include <glm/glm.hpp>

#include <glow/objects/ArrayBufferAttribute.hh>

/// Position is stored separately
struct TerrainVertex
{
    int flags;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            {&TerrainVertex::flags, "aFlags"}, //
        };
    }
};

struct LightVertex
{
    glm::vec3 position;
    float radius;
    glm::vec3 color;
    int seed;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            {&LightVertex::position, "aLightPosition"}, //
            {&LightVertex::radius, "aLightRadius"},     //
            {&LightVertex::color, "aLightColor"},       //
            {&LightVertex::seed, "aLightSeed"},         //
        };
    }
};
