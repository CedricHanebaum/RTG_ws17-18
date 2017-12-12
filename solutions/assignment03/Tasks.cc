/// ===============================================================================================
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
auto paddlePos = paddle.transform->position;
auto targetY = balls[0].transform->position.y;
targetY = glm::clamp(targetY, paddle.shape->halfExtent.y, params.fieldHeight - paddle.shape->halfExtent.y); // paddle cannot leave the field
auto dy = targetY - paddlePos.y;
auto v = paddle.transform->velocity;

auto brakeAccel = maxAccel * glm::sign(-v.y); // always results in braking
auto targetAccel = maxAccel * glm::sign(dy);  // always points toward target

// .. if wrong velocity direction, full brake
if (glm::dot(dy, v.y) < 0.0)
    return brakeAccel;

// calculate if we could reach pos via full brake
auto tZero = glm::abs(v.y) / maxAccel;    // time when zero speed is reached (in full brake)
auto posZero = paddlePos.y + v.y * tZero; // position when zero speed is reached
if (glm::dot(dy, targetY - posZero) < 0.0)
    return brakeAccel; // with full brake we reach our target
else
    return targetAccel; // otherwise go full speed
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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

// target position
auto paddlePos = paddle.transform->position;
auto paddleX = paddlePos.x;
float targetY = params.fieldHeight / 2.0f;

// get ball
auto const& ball = balls[0];
auto r = ball.shape->radius;

// predict location
{
    auto bPos = ball.transform->position;
    auto bVelo = ball.transform->velocity;

    // only if ball is going towards us
    if (glm::dot(bVelo.x, bPos.x - paddleX) < 0)
    {
        auto dx = glm::abs(bPos.x - paddleX) - r - paddle.shape->halfExtent.x;
        auto vx = glm::abs(bVelo.x);
        auto t = dx / vx;
        targetY = bPos.y + bVelo.y * t;

        // consider reflections
        auto minR = r;
        auto maxR = params.fieldHeight - r;
        while (targetY < minR || targetY > maxR)
        {
            if (targetY < minR)
                targetY = 2 * minR - targetY;
            if (targetY > maxR)
                targetY = 2 * maxR - targetY;
        }
    }
}

// try to reach targetY (same as 2a)
// .. clamp it to +- halfExtent
targetY = glm::clamp(targetY, paddle.shape->halfExtent.y, params.fieldHeight - paddle.shape->halfExtent.y);
auto dy = targetY - paddlePos.y;
auto v = paddle.transform->velocity;

auto brakeAccel = maxAccel * glm::sign(-v.y); // always results in braking
auto targetAccel = maxAccel * glm::sign(dy);  // always points toward target

// .. if wrong velocity direction, full brake
if (glm::dot(dy, v.y) < 0.0)
    return brakeAccel;

// calculate if we could reach pos via full brake
auto tZero = glm::abs(v.y) / maxAccel;    // time when zero speed is reached (in full brake)
auto posZero = paddlePos.y + v.y * tZero; // position when zero speed is reached
if (glm::dot(dy, targetY - posZero) < 0.0)
    return brakeAccel; // with full brake we reach our target
else
    return targetAccel; // otherwise go full speed
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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

// target position
auto paddlePos = paddle.transform->position;
auto paddleX = paddlePos.x + paddle.shape->halfExtent.x * glm::sign(params.fieldWidth / 2.0f - paddlePos.x);
float targetY = params.fieldHeight / 2.0f;

// get ball
auto const& ball = balls[0];
auto r = ball.shape->radius;

// predict location (same as 2b)
{
    auto bPos = ball.transform->position;
    auto bVelo = ball.transform->velocity;

    // only if ball is going towards us
    if (glm::dot(bVelo.x, bPos.x - paddleX) < 0)
    {
        auto dx = glm::abs(bPos.x - paddleX) - r;
        auto vx = glm::abs(bVelo.x);
        auto t = dx / vx;
        targetY = bPos.y + bVelo.y * t;

        // consider reflections (-> and also track velocity)
        auto minR = r;
        auto maxR = params.fieldHeight - r;
        while (targetY < minR || targetY > maxR)
        {
            if (targetY < minR)
            {
                targetY = 2 * minR - targetY;
                bVelo.y *= -1.0f;
            }
            if (targetY > maxR)
            {
                targetY = 2 * maxR - targetY;
                bVelo.y *= -1.0f;
            }
        }

        // calculate where to hit
        auto canAdapt = false;
        auto impactPos = glm::vec2(paddleX, targetY);
        for (auto dy = -1; dy <= 1; ++dy) // <- consider hitting via reflection
        {
            auto destinationPos = glm::vec2(params.fieldWidth, params.fieldHeight / 2.0f);
            destinationPos.y += dy * (params.fieldHeight - 2 * r); // reflection

            auto desiredDir = normalize(destinationPos - impactPos);
            auto reflVelo = normalize(bVelo) * glm::vec2(-1, 1);

            auto dotDR = dot(desiredDir, reflVelo);
            auto n = reflVelo - desiredDir * dotDR;
            auto virtualDir = reflVelo - 2 * n;

            if (glm::dot(virtualDir.x, params.fieldWidth / 2.0f - paddleX) < 0)
                continue; // wrong dir

            auto targetT = virtualDir.y / virtualDir.x; // between -1 and 1
            if (targetT >= -1.0f && targetT <= 1.0f)
            {
                // adapt target Y
                auto newTargetY = targetY - paddle.shape->halfExtent.y * targetT;
                // check if can actually be reached in field
                if (newTargetY >= paddle.shape->halfExtent.y
                    && newTargetY <= params.fieldHeight - paddle.shape->halfExtent.y)
                {
                    canAdapt = true;
                    targetY = newTargetY;
                    break;
                }
            }
        }

        // evade otherwise
        if (!canAdapt)
            targetY = fmodf(targetY + params.fieldHeight / 2.0f, params.fieldHeight);
    }
}

// try to reach targetY (same as 2a)
// .. clamp it to +- halfExtent
targetY = glm::clamp(targetY, paddle.shape->halfExtent.y, params.fieldHeight - paddle.shape->halfExtent.y); // paddle cannot leave the field
auto dy = targetY - paddlePos.y;
auto v = paddle.transform->velocity;

auto maxAccelSmooth = maxAccel * (1.0f - 0.99f / (5.0f + glm::abs(dy)));
auto brakeAccel = maxAccelSmooth * glm::sign(-v.y); // always results in braking
auto targetAccel = maxAccelSmooth * glm::sign(dy);  // always points toward target

// .. if wrong velocity direction, full brake
if (glm::dot(dy, v.y) < 0.0)
    return brakeAccel;

// calculate if we could reach pos via full brake
auto tZero = glm::abs(v.y) / maxAccel;    // time when zero speed is reached (in full brake)
auto posZero = paddlePos.y + v.y * tZero; // position when zero speed is reached
if (glm::dot(dy, targetY - posZero) < 0.0)
    return brakeAccel; // with full brake we reach our target
else
    return targetAccel; // otherwise go full speed
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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

// Good AI is a Trade Secret :)

// What the normal AI does:
//  - sort balls by arrival time
//  - for every ball calculate how long each paddle would need to get it
//  - for every ball assign fastest unassigned paddle
//  - now every paddle knows which ball to get (invokes basically 2b)

/// ============= STUDENT CODE END =============
/// ===============================================================================================