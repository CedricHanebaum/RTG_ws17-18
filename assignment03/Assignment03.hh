#pragma once

#include <vector>

#include <glm/ext.hpp>
#include <glm/glm.hpp>

#include <glow/fwd.hh>

#include <glow-extras/glfw/GlfwApp.hh>

#include "Components.hh"
#include "Entity.hh"
#include "Messages.hh"
#include "Parameters.hh"

/**
 * Assignment03: A relatively simple Pong Game written in with the Entity-Component-Systems approach
 */
class Assignment03 : public glow::glfw::GlfwApp
{
private: // logic
    // All constant and transient parameters
    Parameters mParams;

public:
    GLOW_GETTER(Params);

    // Changes the current scenario
    void setScenario(Scenario s);

private:
    // Initializes the game by creating all important entities
    // Also spawn a first ball
    void initGame();

    // Sends a global message to the message queue
    // Messages are handled in processMessages()
    void sendMessage(Message const& msg);

    // Spawns a new ball in the center of the field
    // Does NOT delete the old ball
    void spawnBall();

    // Destroys an entity and all attached components
    // Removes the entity (and its components) from all lists in this class
    void destroyEntity(Entity* entity);

private: // ECS
    // list of entities
    std::vector<SharedEntity> mEntities;

    // list of components
    std::vector<SharedTransformComponent> mTransformComps;
    std::vector<SharedShapeComponent> mShapeComps;
    std::vector<SharedCollisionComponent> mCollisionComps;
    std::vector<SharedRegionDetectorComponent> mRegionDetectorComps;
    std::vector<SharedRenderComponent> mRenderComps;
    std::vector<SharedBallComponent> mBallComps;
    std::vector<SharedPaddleComponent> mPaddleComps;
    std::vector<SharedAIComponent> mAIComps;

    // systems
    void updateMotionSystem(float elapsedSeconds);
    void updateCollisionSystem(float elapsedSeconds);
    void updateRegionDetectorSystem(float elapsedSeconds);
    void updatePaddleSystem(float elapsedSeconds);
    void updateGameLogic(float elapsedSeconds);
    void updateBallSystem(float elapsedSeconds);
    void updateAI(float elapsedSeconds);
    void processMessages();

    // message queue
    std::vector<Message> mMessages;

    // helper
    bool checkSphereSegmentCollision(glm::vec2 v0, glm::vec2 v1, glm::vec2 n, glm::vec2 c, float r, TransformComponent* tc) const;

private: // graphics
    glow::SharedVertexArray mQuad;
    glow::SharedProgram mShaderObj;

public:
    void init() override;
    void update(float elapsedSeconds) override;
    void render(float elapsedSeconds) override;
};
