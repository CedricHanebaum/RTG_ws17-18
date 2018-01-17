#include "Character.hh"

#include <glm/ext.hpp>

#include <glow/common/log.hh>

#include "World.hh"

namespace
{
glm::vec3 getXZPartOfVec3(glm::vec3 const& vec)
{
    auto vecLen = glm::length(vec);
    return vecLen * glm::normalize(glm::vec3(vec.x, 0, vec.z));
}
}
void Character::update(World& world, float elapsedSeconds, glm::vec3 const& movement)
{
    // ensure chunk exists
    {
        auto ip = glm::ivec3(glm::floor(mPosition));
        world.ensureChunkAt(ip);

        if (!world.queryChunk(ip)->isGenerated())
            return; // chunk not generated, do nothing
    }

    RayHit result = world.rayCast(mPosition + glm::vec3(0, mSwimming ? height - swimHeight + 0.1 : maxStepHeight, 0),
                                  glm::vec3(0, -1, 0), mSwimming ? 0.1 : maxStepHeight + 1e-3);
    bool touchesGround = result.hasHit;

    glm::vec2 move(movement.x, movement.z);
    if (glm::length2(move) > 1e-12)
    {
        auto worldSpaceMovement = getXZPartOfVec3(mCam->getInverseRotationMatrix3() * glm::vec3(move.x, 0, move.y));

        // Slower movement when swimming or jumping
        if (mSwimming || !touchesGround)
            worldSpaceMovement *= 0.5f;

        mVelocity = glm::vec3(0, mVelocity.y, 0) + worldSpaceMovement;


        // do not let the character run into walls
        // we actually need to rays (one a little to the left and one to the right)
        // so that we can detect if we walk by a close wall (on the left or right)
        auto leftRay = glm::rotateY(glm::normalize(mVelocity), glm::radians(-25.0f));
        auto rightRay = glm::rotateY(glm::normalize(mVelocity), glm::radians(+25.0f));

        RayHit result = world.rayCast(mPosition + glm::vec3(0, maxStepHeight, 0), getXZPartOfVec3(leftRay), 0.7);
        if (!result.hasHit) // Check the other side
            result = world.rayCast(mPosition + glm::vec3(0, maxStepHeight, 0), getXZPartOfVec3(rightRay), 0.7);

        if (result.hasHit)
        {
            // (solid) block in front of character
            auto prohibitedDir = -glm::vec3(result.hitNormal);
            auto normalAmount = dot(prohibitedDir, mVelocity);
            if (normalAmount > 0)
                mVelocity -= dot(prohibitedDir, mVelocity) * prohibitedDir;
        }
    }
    if (movement.z > 1e-12)
    {
        mVelocity.y += movement.y;
    }


    if (touchesGround)
    {
        auto hitPos = result.hitPos;
        auto inWater = result.block.mat == world.getMaterialFromName("water")->index;
        mSwimming = false;
        if (inWater)
        {
            mSwimming = true;
            hitPos.y += -height + swimHeight; // eyes 30cm above water surface
        }

        // Prevent player from running into hills
        if (hitPos.y - 0.1 > mPosition.y)
            mVelocity *= 0;

        // Time in seconds until 80% of the position is updated
        const auto tau = 0.05f;
        mPosition = glm::mix(hitPos, mPosition, glm::pow(0.2, elapsedSeconds / tau));
        mVelocity.y = movement.y;
    }
    else
    {
        // Character is not touching ground
        auto acceleration = glm::vec3(0, -9.80665, 0);
        mVelocity += elapsedSeconds * acceleration;
    }

    // update position
    mPosition += elapsedSeconds * mVelocity;

    // remove velocity within xz plane;
    mVelocity.x = 0;
    mVelocity.z = 0;

    // get camera position from character position
    mCam->setPosition(mPosition + glm::vec3(0, height, 0));
}
