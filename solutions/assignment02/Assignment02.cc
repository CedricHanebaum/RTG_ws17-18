/// ===============================================================================================
/// Task 1.a
/// The motion system is responsible for updating the position of all transform components based on their velocity
///
/// The transform components are stored in the vector 'mTransformComps' (see Assignment02.hh)
/// For the definition of a transform component see Components.hh
///
/// Your job is to:
///     - iterate over these components
///     - apply the motion update: x_{i+1} = x_i + v_i * dt
///
/// Note:
///     - iterating over a C++ vector v can be done in two ways:
///         for (auto i = 0; i < v.size(); ++i) // i is an int
///         {
///             auto const& element = v[i]; // element is an immutable reference to the i'th entry
///         }
///         // or directly:
///         for (auto const& element : v) { ... }
///
/// ============= STUDENT CODE BEGIN =============
for (auto const& transformComp : mTransformComps)
{
    transformComp->position += transformComp->velocity * elapsedSeconds;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
/// Task 1.b
/// Right now the ball is missing a dynamic collision component
///
/// Your job is to:
///     - add the collision component to the entity
///     - set it to dynamic
///     - add it to the global list of collision components
///
/// Note:
///     - you can take inspiration from this function and initGame()
///
/// ============= STUDENT CODE BEGIN =============
auto cc = ball->addComponent<CollisionComponent>();
cc->dynamic = true;
mCollisionComps.push_back(cc);
/// ============= STUDENT CODE END =============
/// ===============================================================================================


        
/// ===============================================================================================        
/// Task 1.c
/// The next step is to add input handling
///
/// Your job is to:
///     - determine if the paddle should be moved up or down
///     - change transform->velocity of the paddle depending on the input
///     - guarantee that the paddle cannot leave the game field (not even partially)
///
/// Note:
///     - Player::Left should use W/S for up/down
///     - Player::Right should use UP/DOWN (arrows) for up/down
///     - you can check if a key is pressed via 'isKeyPressed(GLFW_KEY_<Name of the Key>)'
///     - the paddle speed is stored in 'speed'
///     - the game field starts at y = 0 and goes up to y = mFieldHeight
///     - check out Components.hh if you want to know what data you can access
///     - use transform->velocity to move the paddle and transform->position to prevent it from leaving
///
/// ============= STUDENT CODE BEGIN =============
auto moveUp = false;
auto moveDown = false;

switch (paddle->owner)
{
case Player::Left:
    moveUp = isKeyPressed(GLFW_KEY_W);
    moveDown = isKeyPressed(GLFW_KEY_S);
    break;
case Player::Right:
    moveUp = isKeyPressed(GLFW_KEY_UP);
    moveDown = isKeyPressed(GLFW_KEY_DOWN);
    break;
}

auto vy = 0.0f;
if (moveUp)
    vy += speed;
if (moveDown)
    vy -= speed;

transform->velocity = {0, vy};
transform->position.y = glm::clamp(transform->position.y, shape->halfExtent.y, mFieldHeight - shape->halfExtent.y);
/// ============= STUDENT CODE END =============
/// ===============================================================================================

    

/// ===============================================================================================
/// Task 1.d
/// The region detectors are HalfPlaneShapes that should send a message
/// when a dynamic collision component is completely inside the detector
///
/// Your job is to:
///     - check if the sphere is completely (not partially) inside the detector region
///     - send a region detection message if the sphere is detected
///
/// Note:
///     - we statically assume that the dynamic shape is a SphereShape
///     - sending messages works via 'sendMessage({MessageType::<your type>, <sender>, <subject>})'
///     - check Messages.hh for details on the message struct
///
/// ============= STUDENT CODE BEGIN =============
auto dis = dot(dynamicTransform->position - detectorPos, halfPlaneShape->normal);
if (dis < -dynamicShape->radius)
{
    // Detection event
    sendMessage({MessageType::RegionDetection, detectorComp, dynamicComp});
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
/// Task 1.e
/// When a ball is in the goal region, a region detection message is sent
/// This indicates that one player scored a point
///
/// Your job is to:
///     - check if message subject is a ball
///     - increase 'mScoreLeft' or 'mScoreRight' depending on the owner of the region detector
///     - destroy the ball
///     - spawn a new ball (only if there are no current balls)
///
/// Note:
///     - an entity is a ball if it has a ball component
///     - see Entity.hh for how to access/check components
///     - see Messages.hh for how to interpret its content (the current message is 'msg')
///     - see Assignment02.hh for useful functions and members
///     - all active ball components are stored in 'mBallComps' (you can check if a vector is '.empty()')
///     - you can also write to the console via 'glow::info() << "...";' when a player scored
///
/// ============= STUDENT CODE BEGIN =============
auto detector = msg.sender->entity->getComponent<RegionDetectorComponent>();
if (msg.subject->entity->hasComponent<BallComponent>())
{
    // keep score
    switch (detector->owner)
    {
    case Player::Left:
        glow::info() << "Right Player Scored!";
        mScoreRight++;
        break;

    case Player::Right:
        glow::info() << "Left Player Scored!";
        mScoreLeft++;
        break;
    }

    // destroy ball
    destroyEntity(msg.subject->entity);

    // spawn new ball if last ball was destroyed
    if (mBallComps.empty())
        spawnBall();
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================
            


/// ===============================================================================================
/// Task 2.a
/// Without a proper spawn velocity the ball will only move horizontally
///
/// Your job is to:
///     - rotate the velocity by -20°..20° (uniformly, 0° is horizontal)
///     - choose the speed randomly between 300..400
///     - decide if the ball goes left or right (50/50 randomly)
///
/// Note:
///     - see random(<min>, <max>) (in this file) - don't use RAND()
///
/// ============= STUDENT CODE BEGIN =============
// +- 20° angle
auto angle = glm::radians(random(-20., 20.));
spawnVelocity = {
    glm::cos(angle), //
    glm::sin(angle), //
};

// speed between 300 and 400
spawnVelocity *= random(300, 400);

// random dir
if (random(0, 1) < 0.5)
    spawnVelocity *= -1.0f;
/// ============= STUDENT CODE END =============
/// ===============================================================================================
        
        

/// ===============================================================================================
/// Task 2.b
/// Collision events are generated when a dynamic object hits a static one
/// We want to spice up gameplay by accelerating the ball by 20% every time a _paddle_ is hit
///
/// Your job is to:
///     - check if the collision is between a paddle and a ball
///     - increase the ball velocity by 20%
///     - if the speed is above speedLimit, set 'isAtSpeedLimit' to true and clamp the ball velocity
///
/// Note:
///     - again, check Messages.hh and Entity.hh for useful info/functions
///
/// ============= STUDENT CODE BEGIN =============
// accelerate if paddle is hit
if (msg.sender->entity->hasComponent<PaddleComponent>() && //
    msg.subject->entity->hasComponent<BallComponent>())
{
    auto tc = msg.subject->entity->getComponent<TransformComponent>();

    auto speed = length(tc->velocity);
    speed *= 1.2f;

    if (speed > speedLimit)
    {
        speed = speedLimit;
        isAtSpeedLimit = true;
    }

    tc->velocity = speed * normalize(tc->velocity);
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
/// Task 2.c
/// As a last improvement to gameplay we want to spawn additional balls
/// if a paddle is hit at the speed limit and no more than one every 5 seconds
///
/// Your job is to:
///     - spawn a new ball (at most one every 5s)
///
/// Note:
///     - you may use 'mMultiBallCooldown' (a float decreasing by 1 per sec and starts at 0)
///
/// ============= STUDENT CODE BEGIN =============
// spawn another ball if out of Cooldown and hit at max speed
if (mMultiBallCooldown < 0.0f)
{
    spawnBall();
    mMultiBallCooldown = 5.0f;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================