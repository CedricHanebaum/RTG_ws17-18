#pragma once

#include <cstddef>
#include <vector>

#include "Parameters.hh"
#include "Player.hh"

#include <glm/ext.hpp>

struct TransformComponent;
struct SphereShapeComponent;
struct BoxShapeComponent;

namespace ai
{
struct Ball
{
    const TransformComponent* const transform;
    const SphereShapeComponent* const shape;
};

struct Paddle
{
    Player const owner;
    const TransformComponent* const transform;
    const BoxShapeComponent* const shape;
};

/**
 * @param paddleIdx   index of paddle on the current side (0, 1, 2, ...)
 * @param paddle      the current paddle
 * @param allPaddles  a list of all paddles (includes current and opponents)
 * @param balls       a list of all active balls (guaranteed to be non-empty)
 * @param params      a set of all transient and constant simulation parameters
 * @return the new desired acceleration
 */
float simpleAI(size_t paddleIdx,
               Paddle const& paddle,
               std::vector<Paddle> const& allPaddles,
               std::vector<Ball> const& balls,
               Parameters const& params,
               float elapsedSeconds,
               glm::vec3& debugColor);
float normalAI(size_t paddleIdx,
               Paddle const& paddle,
               std::vector<Paddle> const& allPaddles,
               std::vector<Ball> const& balls,
               Parameters const& params,
               float elapsedSeconds,
               glm::vec3& debugColor);
float goodAI(size_t paddleIdx,
             Paddle const& paddle,
             std::vector<Paddle> const& allPaddles,
             std::vector<Ball> const& balls,
             Parameters const& params,
             float elapsedSeconds,
             glm::vec3& debugColor);

float task2a(size_t paddleIdx,
             Paddle const& paddle,
             std::vector<Paddle> const& allPaddles,
             std::vector<Ball> const& balls,
             Parameters const& params,
             float elapsedSeconds,
             glm::vec3& debugColor);
float task2b(size_t paddleIdx,
             Paddle const& paddle,
             std::vector<Paddle> const& allPaddles,
             std::vector<Ball> const& balls,
             Parameters const& params,
             float elapsedSeconds,
             glm::vec3& debugColor);
float task2c(size_t paddleIdx,
             Paddle const& paddle,
             std::vector<Paddle> const& allPaddles,
             std::vector<Ball> const& balls,
             Parameters const& params,
             float elapsedSeconds,
             glm::vec3& debugColor);
float task3(size_t paddleIdx,
            Paddle const& paddle,
            std::vector<Paddle> const& allPaddles,
            std::vector<Ball> const& balls,
            Parameters const& params,
            float elapsedSeconds,
            glm::vec3& debugColor);
}
