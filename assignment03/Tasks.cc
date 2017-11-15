#include "AI.hh"

#include <algorithm>
#include <cassert>

#include <glow/common/log.hh>

#include <glm/ext.hpp>

#include "Components.hh"

/// Note: Entity is NOT included.
/// You are not allowed to change any component
/// All information is given and the only side effect should be the return value

using namespace ai;

float ai::simpleAI(size_t paddleIdx,
                   const Paddle& paddle,
                   const std::vector<Paddle>& allPaddles,
                   const std::vector<Ball>& balls,
                   const Parameters& params,
                   float elapsedSeconds,
                   glm::vec3& debugColor)
{
    auto maxAccel = params.paddleMaxAcceleration;

    // brake acceleration always reduces velocity
    auto brakeAccel = maxAccel * glm::sign(-paddle.transform->velocity.y);

    if (paddleIdx >= balls.size())
        return brakeAccel; // we don't have any ball -> brake

    // i-th paddle takes i-th ball
    auto ballY = balls[paddleIdx].transform->position.y;

    auto dy = ballY - paddle.transform->position.y;
    dy -= glm::min(paddle.shape->halfExtent.y * 0.6f, glm::abs(dy)) * glm::sign(dy);

    if (glm::abs(paddle.transform->velocity.y) < 0.1) // accelerate towards dy if no velocity
    {
        debugColor = {1, 1, 1}; // no velocity? -> black
        return maxAccel * glm::sign(dy);
    }

    auto arrivalTime = dy / paddle.transform->velocity.y;
    auto brakeTime = glm::abs(paddle.transform->velocity.y) / maxAccel * 1.5f; // 50% braking margin

    auto hasVelo = glm::abs(paddle.transform->velocity.y) > 400; // used to prevent flickering at target

    if (brakeTime < arrivalTime) // we have time to brake -> we can accelerate towards dy
    {
        debugColor = hasVelo ? glm::vec3(0, 1, 0) : glm::vec3(1, 1, 1); // accelerating -> green
        return maxAccel * glm::sign(dy);
    }
    else // .. otherwise brake
    {
        debugColor = hasVelo ? glm::vec3(1, 0, 0) : glm::vec3(1, 1, 1); // braking -> red
        return brakeAccel;
    }
}

float ai::task2a(size_t paddleIdx,
                 Paddle const& paddle,
                 std::vector<Paddle> const& allPaddles,
                 std::vector<Ball> const& balls,
                 Parameters const& params,
                 float elapsedSeconds,
                 glm::vec3& debugColor)
{
    /// Task 2.a
    /// For the paddle with index "paddleIdx", the paddle position
    /// can be controlled only indirectely by returning an acceleration value.
    /// You cannot set any component properties directly.
    ///
    /// Your job is to:
    ///     - accelerate the paddle so that it always collides with the ball
    ///     - make sure the ball hits almost exactly the paddle center
    ///
    /// Notes:
    ///     - you can assume the paddle to be the left one
    ///     - the ball will always move horizontally from right to left
    ///     - you can slow down if the acceleration and velocity go in opposite directions
    ///     - the return value is clamped to +- params.paddleMaxAcceleration
    ///     - have a look at the console output to see where the ball hit the paddle
    ///       (It should be between 45% and 55% except at field boundaries)
    ///     - For debugging purposes, you can set the debugColor of the paddle
    ///     - when the ball is moving away from you we recommend returning to the center
    ///
    /// ============= STUDENT CODE BEGIN =============

    auto maxAccel = params.paddleMaxAcceleration;
    return maxAccel;

    /// ============= STUDENT CODE END =============
}
float ai::task2b(size_t paddleIdx,
                 Paddle const& paddle,
                 std::vector<Paddle> const& allPaddles,
                 std::vector<Ball> const& balls,
                 Parameters const& params,
                 float elapsedSeconds,
                 glm::vec3& debugColor)
{
    /// Task 2.b
    /// Here, wall reflections have to be taken into account,
    /// i.e. the ball does not only move horizontally.
    ///
    /// Your job is to:
    ///     - predict the ball trajectory incl. all reflections
    ///     - return the acceleration the paddle needs to reach the predicted position in time
    ///     - make sure the ball hits almost exactly the paddle center (exception: field boundary)
    ///
    /// Notes:
    ///     - don't forget the paddle extent
    ///     - don't forget the ball radius when predicting reflections
    ///     - you can use `glow::info() << targetY;` to see if your prediction is stable
    ///     - when the ball is moving away from you we recommend returning to the center
    ///
    /// ============= STUDENT CODE BEGIN =============

    auto maxAccel = params.paddleMaxAcceleration;
    return maxAccel;

    /// ============= STUDENT CODE END =============
}
float ai::task2c(size_t paddleIdx,
                 Paddle const& paddle,
                 std::vector<Paddle> const& allPaddles,
                 std::vector<Ball> const& balls,
                 Parameters const& params,
                 float elapsedSeconds,
                 glm::vec3& debugColor)
{
    /// Task 2.c
    /// The paddle should be able to control where the ball arrives on the other side.
    /// Here, every reflected ball should arive at the center of the right side.
    ///
    /// Your job is to:
    ///     - Hit the ball so that its destination is the center of the right side
    ///     - if it is impossible to reflect the ball towards the right center, do not hit it at all
    ///
    /// Notes:
    ///     - this tasks exploits the mechanic that you implemented in 1.b
    ///     - don't forget the paddle extent
    ///     - don't forget the ball radius when predicting reflections
    ///     - you may have to use a reflection to hit the opposite center
    ///     - when the ball is moving away from you we recommend returning to the center
    ///
    /// ============= STUDENT CODE BEGIN =============

    auto maxAccel = params.paddleMaxAcceleration;
    return maxAccel;

    /// ============= STUDENT CODE END =============
}

float ai::task3(size_t paddleIdx,
                Paddle const& paddle,
                std::vector<Paddle> const& allPaddles,
                std::vector<Ball> const& balls,
                Parameters const& params,
                float elapsedSeconds,
                glm::vec3& debugColor)
{
    /// Task 3
    /// Try to direct the ball so that the enemy AI (right paddle) has a hard time reaching it!
    ///
    /// Your job is to:
    ///     - write an artificial intelligence that beats the enemy AI
    ///
    /// Notes:
    ///     - you actually control three paddles, but this method is called
    ///       once for every one of those (with different paddleIdx and paddle)
    ///     - your AI must be fair, i.e. the return value of this method is
    ///       the only way to change the global state (no memhacks, no calling our AI, etc.)
    ///     - you may take a look at simpleAI(). It should be easy to beat if you've done 2a-2c.
    ///
    /// ============= STUDENT CODE BEGIN =============

    auto maxAccel = params.paddleMaxAcceleration;
    return maxAccel;

    /// ============= STUDENT CODE END =============
}
