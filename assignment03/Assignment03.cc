#include "Assignment03.hh"

#include <ctime>

// OpenGL header
#include <glow/gl.hh>

// Glow helper
#include <glow/common/log.hh>
#include <glow/common/str_utils.hh>

// used OpenGL object wrappers
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/Program.hh>
#include <glow/objects/Texture2D.hh>
#include <glow/objects/VertexArray.hh>

// AntTweakBar
#include <AntTweakBar.h>

// GLFW
#include <GLFW/glfw3.h>

// extra helper
#include <glow-extras/geometry/Quad.hh>
#include <glow-extras/timing/PerformanceTimer.hh>
#include "Helper.hh"

#include "AI.hh"

// in the implementation, we want to omit the glow:: prefix
using namespace glow;

// returns a uniform random float within min and max
static float random(float min, float max);

///
/// GLM Primer:
/// glm:: contains basic vector math functions and classes
/// it closely mimics the naming of GLSL
///
/// Vectors:
///     glm::vec2 v = {1.0f, 0.5f};
///
/// Normal math:
///     glm::vec2 a, b;
///     float f;
///     a + b
///     a * f
///
/// Component access:
///     v.x = 3.0f;
///
/// Useful functions:
///     dot(a, b)    // inner product
///     length(a)    // 2-norm
///     normalize(a) // a / |a|
///     glm::radians(50.0f)      // converts radians to degree
///     glm::cos(f), glm::sin(f) // sin/cos in radians
///

void Assignment03::setScenario(Scenario s)
{
    // Configure parameters
    mParams = Parameters();
    mParams.scenario = s;
    switch (s)
    {
    case Scenario::Task2A:
    case Scenario::Task2B:
    case Scenario::Task2C:
        mParams.ballAcceleration = 0.0f;
        mParams.ballDrag = 0.0f;
        mParams.ballStartSpeed = 1500.0f;
        mParams.ballStartAngle = 75.0f;
        mParams.paddlesLeft = 1;
        mParams.paddlesRight = 0;
        mParams.multiBallTime = 1e10; // no multi balls
        break;
    case Scenario::Task3:
        break;
    }

    // clear all entities
    mEntities.clear();

    // clear all comps
    mTransformComps.clear();
    mShapeComps.clear();
    mCollisionComps.clear();
    mRegionDetectorComps.clear();
    mRenderComps.clear();
    mBallComps.clear();
    mPaddleComps.clear();
    mAIComps.clear();

    // init game again
    initGame();

    TwRefreshBar(tweakbar());
}

void Assignment03::initGame()
{
    // Ball entity
    spawnBall();

    // Paddles have
    // - a transform component
    // - a render component
    // - a box shape component (20 x 120)
    // - a static collision component
    // - a paddle component (owned by the respective player)
    for (auto player : {Player::Left, Player::Right})
    {
        auto pCnt = player == Player::Left ? mParams.paddlesLeft : mParams.paddlesRight;
        for (auto i = 0; i < pCnt; ++i)
        {
            const int paddleMargin = 30 + 25 * (pCnt - 1 - i);
            auto paddle = std::make_shared<Entity>(player == Player::Left ? "Left Paddle" : "Right Paddle");

            auto tc = paddle->addComponent<TransformComponent>();
            tc->position = glm::vec2(player == Player::Left ? paddleMargin : mParams.fieldWidth - paddleMargin,
                                     mParams.fieldHeight * (i + 0.5f) / 4.0f);
            mTransformComps.push_back(tc);

            auto rc = paddle->addComponent<RenderComponent>();
            rc->color = glm::vec3(1, 1, 1);
            mRenderComps.push_back(rc);

            auto sc = paddle->addComponent<BoxShapeComponent>();
            sc->halfExtent = {10, 60};
            mShapeComps.push_back(sc);

            auto cc = paddle->addComponent<CollisionComponent>();
            cc->dynamic = false;
            mCollisionComps.push_back(cc);

            auto pc = paddle->addComponent<PaddleComponent>();
            pc->owner = player;
            mPaddleComps.push_back(pc);

            // both player get AI
            auto ai = paddle->addComponent<AIComponent>();
            mAIComps.push_back(ai);

            mEntities.push_back(paddle);
        }
    }

    // Game borders have
    // - a transform component
    // - a half plane shape
    // - a static collision component
    for (auto isTop : {true, false})
    {
        auto border = std::make_shared<Entity>(isTop ? "Top Border" : "Bottom Border");

        auto tc = border->addComponent<TransformComponent>();
        tc->position = glm::vec2(mParams.fieldWidth / 2, isTop ? 0 : mParams.fieldHeight);
        mTransformComps.push_back(tc);

        auto cc = border->addComponent<CollisionComponent>();
        cc->dynamic = false;
        mCollisionComps.push_back(cc);

        auto sc = border->addComponent<HalfPlaneShapeComponent>();
        sc->normal = glm::vec2(0, isTop ? 1 : -1);
        mShapeComps.push_back(sc);

        mEntities.push_back(border);
    }

    // Region detector have
    // - a transform component
    // - a half plane shape
    // - a region detector component (owned by the respective player)
    for (auto player : {Player::Left, Player::Right})
    {
        auto detector = std::make_shared<Entity>(player == Player::Left ? "Left Detector" : "Right Detector");

        auto tc = detector->addComponent<TransformComponent>();
        tc->position = glm::vec2(player == Player::Left ? 0 : mParams.fieldWidth, mParams.fieldHeight / 2);
        mTransformComps.push_back(tc);

        auto rdc = detector->addComponent<RegionDetectorComponent>();
        rdc->owner = player;
        mRegionDetectorComps.push_back(rdc);

        auto sc = detector->addComponent<HalfPlaneShapeComponent>();
        sc->normal = glm::vec2(player == Player::Left ? 1 : -1, 0);
        mShapeComps.push_back(sc);

        mEntities.push_back(detector);
    }
}

void Assignment03::spawnBall()
{
    auto ball = std::make_shared<Entity>("Ball");

    // Balls have
    // - a transform component (starts in center with spawnVelocity)
    // - a render component
    // - a sphere shape component (radius 15)
    // - a dynamic collision component
    // - a ball component

    glm::vec2 spawnVelocity = {300, 0}; // dummy initial value
    glm::vec2 spawnPos = glm::vec2(mParams.fieldWidth, mParams.fieldHeight) / 2.0f;
    auto angle = glm::radians(random(-mParams.ballStartAngle, mParams.ballStartAngle));
    switch (mParams.scenario)
    {
    case Scenario::Task2A: // random horizontal ball
        spawnPos = glm::vec2(mParams.fieldWidth, mParams.fieldHeight * random(0.1f, 0.9f));
        spawnVelocity = glm::vec2(-mParams.ballStartSpeed, 0) * random(0.8f, 1.2f);
        break;

    case Scenario::Task2C: // random ball
    case Scenario::Task2B:
        spawnPos = glm::vec2(mParams.fieldWidth, mParams.fieldHeight * random(0.1f, 0.9f));
        spawnVelocity = glm::vec2(glm::cos(angle), glm::sin(angle)) * random(0.8f, 1.2f) * -mParams.ballStartSpeed;
        break;

    default:
        // +- 30° angle
        spawnVelocity = {
            glm::cos(angle), //
            glm::sin(angle), //
        };

        // speed
        spawnVelocity *= mParams.ballStartSpeed * random(0.8f, 1.2f);

        // random dir
        if (random(0, 1) < 0.5)
            spawnVelocity *= -1.0f;
        break;
    }

    auto tc = ball->addComponent<TransformComponent>();
    tc->position = spawnPos;
    tc->velocity = spawnVelocity;
    tc->linearDrag = mParams.ballDrag;
    mTransformComps.push_back(tc);

    auto rc = ball->addComponent<RenderComponent>();
    rc->color = glm::vec3(.89f, .00f, .40f);
    mRenderComps.push_back(rc);

    auto sc = ball->addComponent<SphereShapeComponent>();
    sc->radius = 15;
    mShapeComps.push_back(sc);

    auto bc = ball->addComponent<BallComponent>();
    mBallComps.push_back(bc);

    auto cc = ball->addComponent<CollisionComponent>();
    cc->dynamic = true;
    mCollisionComps.push_back(cc);

    mEntities.push_back(ball);

    // reset multi ball cooldown
    mParams.multiBallCooldown = mParams.multiBallTime;

    // show new ball
    TwRefreshBar(tweakbar());
}

void Assignment03::updateMotionSystem(float elapsedSeconds)
{
    for (auto const& transformComp : mTransformComps)
    {
        /// Task 1.a
        /// Until now, TransformComponents contained only position and velocity.
        /// Now, paddles are moved by setting the acceleration
        /// (and updating velocity and position accordingly).
        /// Furthermore, linear drag is introduced to simulate air friction
        /// that "dampens" the acceleration based on the current velocity.
        ///
        /// Your job is to:
        ///     - apply linear drag to the acceleration
        ///     - update the velocity
        ///
        /// Notes:
        ///     - see Components.hh for the definition of a transform component
        ///     - you should not change transformComp->acceleration (linear drag is only added temporarily!)
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============

        transformComp->position += transformComp->velocity * elapsedSeconds;
    }
}

bool Assignment03::checkSphereSegmentCollision(glm::vec2 p0, glm::vec2 p1, glm::vec2 n, glm::vec2 c, float r, TransformComponent* tc) const
{
    assert(p0 != p1);

    auto isCollision = false;

    // Sphere-Line-Intersection:

    // dir = v1 - v0
    // l = v0 + dir * t
    // sdir = l - c
    // <sdir, dir> == 0
    // <v0 + dir * t - c, dir> == 0
    // <v0 - c, dir> + t * <dir, dir> == 0
    // t = - <v0 - c, dir> / <dir, dir>
    auto d10 = p1 - p0;
    auto d0c = p0 - c;
    auto t = -dot(d0c, d10) / dot(d10, d10);
    t = glm::clamp(t, 0.0f, 1.0f);
    auto np = p0 + d10 * t;
    auto dis = distance(np, c);

    if (dis < r)
    {
        isCollision = true;
    }

    // reflection
    if (isCollision)
    {
        auto dotVN = dot(n, tc->velocity);
        if (dotVN < 0)
        {
            tc->velocity -= 2.0f * dotVN * n;
            assert(dot(tc->velocity, n) > 0);

            // t from 0..1 indicates relative collision position
            // induces a half-angle shift towards -45° .. +45°

            assert(p0.y < p1.y);

            /// Task 1.b
            /// The reflection direction should depend on where exactly
            /// the ball hits the paddle to allow more control over the ball.
            ///
            /// Your job is to:
            ///     - compute a guiding vector that points 45° (top-right) at the paddle top,
            ///       -45° (bottom-right) at the paddle bottom,
            ///       0° at the paddle center and interpolate linearly everywhere in between
            ///       (more precise: if `a` is linearly interpolated between -1 (bottom) and 1 (top),
            ///        then the guiding vector is normalize(vec2(n.x, a)) - n is the paddle normal)
            ///     - set velocity to the halfway vector between the guiding vector and the reflection vector
            ///       (the new velocity should have the same length as before
            ///        and the angle between new velocity and guide should equal new velocity and reflection)
            ///
            /// Notes:
            ///     - tc->velocity is the reflected velocity
            ///     - n is the collision normal
            ///     - t is the relative y-coord of the (local) collision point
            ///       ranging from 0 (bottom) to 1 (top) of the paddle
            ///     - the functionality of this code can be tested in task 2.a
            ///     - the speed (i.e. the length of the velocity vector) must not change
            ///
            /// ============= STUDENT CODE BEGIN =============

            /// ============= STUDENT CODE END =============

            if (mParams.scenario != Scenario::Task3)
            {
                glow::info() << "Paddle was hit at " << t * 100 << "% of height (y = " << c.y << ")";
            }
        }
        else
            isCollision = false; // ignore wrong direction
    }

    return isCollision;
}

void Assignment03::updateCollisionSystem(float elapsedSeconds)
{
    for (auto const& dynamicComp : mCollisionComps)
        if (dynamicComp->dynamic)
            for (auto const& staticComp : mCollisionComps)
                if (!staticComp->dynamic)
                {
                    // sphere only
                    auto dynamicShape = dynamicComp->entity->getComponent<SphereShapeComponent>();
                    auto dynamicTransform = dynamicComp->entity->getComponent<TransformComponent>();

                    auto staticShape = staticComp->entity->getComponent<ShapeComponent>();
                    auto staticPos = staticComp->entity->getComponent<TransformComponent>()->position;
                    auto staticBoxShape = dynamic_cast<BoxShapeComponent*>(staticShape);
                    auto staticHalfPlaneShape = dynamic_cast<HalfPlaneShapeComponent*>(staticShape);
                    if (staticBoxShape != nullptr)
                    {
                        // Only the "front" of a box collides
                        auto dir = glm::sign(mParams.fieldWidth / 2 - staticPos.x);
                        auto yMin = staticPos.y - staticBoxShape->halfExtent.y;
                        auto yMax = staticPos.y + staticBoxShape->halfExtent.y;
                        auto x = staticPos.x + staticBoxShape->halfExtent.x * dir;
                        auto c = dynamicTransform->position;
                        auto r = dynamicShape->radius;
                        auto v = dynamicTransform->velocity;
                        auto nv = normalize(v);
                        // performs simple CCD (continuous collision detection) w.r.t. the ball
                        // if distance travelled in one timestep is > r * 0.1, perform substeps
                        auto dis = length(v) * elapsedSeconds;
                        for (auto f = 0.0f; f < dis; f += r * 0.1f)
                            if (checkSphereSegmentCollision({x, yMin}, {x, yMax}, {dir, 0}, c + nv * f, r, dynamicTransform))
                            {
                                // Collision event
                                sendMessage({MessageType::Collision, staticComp, dynamicComp});

                                // Reflection is already done

                                // improve position of ball a bit
                                dynamicTransform->position = c + nv * f - normalize(dynamicTransform->velocity) * f;

                                // stop CCD
                                break;
                            }
                    }
                    else if (staticHalfPlaneShape != nullptr)
                    {
                        auto dis = dot(dynamicTransform->position - staticPos, staticHalfPlaneShape->normal);
                        if (dis < dynamicShape->radius)
                        {
                            // Collision event
                            sendMessage({MessageType::Collision, staticComp, dynamicComp});

                            // Reflection
                            auto dotVN = dot(staticHalfPlaneShape->normal, dynamicTransform->velocity);
                            if (dotVN < 0)
                            {
                                dynamicTransform->velocity -= 2.0f * dotVN * staticHalfPlaneShape->normal;

                                // .. and more precise position
                                dynamicTransform->position += staticHalfPlaneShape->normal * 2 * (dynamicShape->radius - dis);
                            }
                        }
                    }
                    else
                        glow::error() << "Unsupported static shape";
                }
}

void Assignment03::updateRegionDetectorSystem(float elapsedSeconds)
{
    for (auto const& detectorComp : mRegionDetectorComps)
        for (auto const& dynamicComp : mCollisionComps)
            if (dynamicComp->dynamic)
            {
                // dynamic can be sphere only
                auto dynamicShape = dynamicComp->entity->getComponent<SphereShapeComponent>();
                auto dynamicTransform = dynamicComp->entity->getComponent<TransformComponent>();

                // detector is half plane only
                auto halfPlaneShape = detectorComp->entity->getComponent<HalfPlaneShapeComponent>();
                auto detectorPos = detectorComp->entity->getComponent<TransformComponent>()->position;

                auto dis = dot(dynamicTransform->position - detectorPos, halfPlaneShape->normal);
                if (dis < -dynamicShape->radius)
                {
                    // Detection event
                    sendMessage({MessageType::RegionDetection, detectorComp, dynamicComp});
                }
            }
}

void Assignment03::updatePaddleSystem(float elapsedSeconds)
{
    for (auto const& paddle : mPaddleComps)
    {
        auto transform = paddle->entity->getComponent<TransformComponent>();
        auto shape = paddle->entity->getComponent<BoxShapeComponent>();

        // limit paddle acceleration (set by AI)
        if (length(transform->acceleration) > mParams.paddleMaxAcceleration)
            transform->acceleration = normalize(transform->acceleration) * mParams.paddleMaxAcceleration;

        // paddle collisions
        {
            auto collisionDampening = 1 - mParams.paddleCollisionDampening;

            // .. with border (last because it has priority)
            if (transform->position.y < shape->halfExtent.y)
            {
                transform->position.y = shape->halfExtent.y;
                if (transform->velocity.y < 0)
                    transform->velocity *= -collisionDampening;
            }
            if (transform->position.y > mParams.fieldHeight - shape->halfExtent.y)
            {
                transform->position.y = mParams.fieldHeight - shape->halfExtent.y;
                if (transform->velocity.y > 0)
                    transform->velocity *= -collisionDampening;
            }
        }
    }
}

void Assignment03::updateGameLogic(float elapsedSeconds)
{
    // The only job of our global game logic system is to count down the cooldown for multi balls
    // No need to clamp against zero
    mParams.multiBallCooldown -= elapsedSeconds;

    // spawn ball (resets cooldown)
    if (mParams.multiBallCooldown < 0.0f)
        spawnBall();
}

void Assignment03::updateBallSystem(float elapsedSeconds)
{
    for (auto const& ball : mBallComps)
    {
        auto transform = ball->entity->getComponent<TransformComponent>();

        // accelerate balls towards v.x
        auto dir = random(0.0f, 1.0f) < 0.5f ? 1.0f : -1.0f;
        if (transform->velocity.x > 0)
            dir = 1.0f;
        if (transform->velocity.x < 0)
            dir = -1.0f;

        transform->acceleration = {dir * mParams.ballAcceleration, 0.0f};

        // velocity is capped by bounce dampening
    }
}

void Assignment03::updateAI(float elapsedSeconds)
{
    using namespace ai;

    std::vector<Ball> balls;
    std::vector<Paddle> paddles;
    for (auto const& b : mBallComps)
        balls.push_back({b->entity->getComponent<TransformComponent>(), //
                         b->entity->getComponent<SphereShapeComponent>()});
    for (auto const& p : mPaddleComps)
        paddles.push_back({p->owner,                                      //
                           p->entity->getComponent<TransformComponent>(), //
                           p->entity->getComponent<BoxShapeComponent>()});
    auto getPaddle = [&](Entity* e) -> Paddle {
        for (auto const& p : paddles)
            if (p.transform->entity == e)
                return p;
        throw "Paddle Not Found";
    };

    for (auto const& aic : mAIComps)
    {
        auto entity = aic->entity;
        if (entity->hasComponent<PaddleComponent>())
        {
            // get current paddle and its idx
            auto currPaddle = getPaddle(entity);
            auto pIdx = 0u;
            for (auto const& p : paddles)
                if (p.transform->entity == entity)
                    break;
                else if (p.owner == currPaddle.owner)
                    ++pIdx;

            // execute AI
            glm::vec3 debugColor = {1, 1, 1};
            float accel = 0.0f;
            auto owner = entity->getComponent<PaddleComponent>()->owner;
            switch (mParams.scenario)
            {
            case Scenario::Task2A:
                accel = ai::task2a(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                break;
            case Scenario::Task2B:
                accel = ai::task2b(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                break;
            case Scenario::Task2C:
                accel = ai::task2c(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                break;
            case Scenario::Task3:
                if (owner == Player::Left)
                    accel = ai::task3(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                else
                    switch (mParams.enemy)
                    {
                    case EnemyAI::Simple:
                        accel = ai::simpleAI(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                        break;
                    case EnemyAI::Normal:
                        accel = ai::normalAI(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                        break;
                    case EnemyAI::Good:
                        accel = ai::goodAI(pIdx, currPaddle, paddles, balls, mParams, elapsedSeconds, debugColor);
                        break;
                    }
                break;
            }
            assert(std::isfinite(accel));

            // set debug color
            entity->getComponent<RenderComponent>()->color = debugColor;

            // clamp and set acceleration
            accel = glm::clamp(accel, -mParams.paddleMaxAcceleration, mParams.paddleMaxAcceleration);
            entity->getComponent<TransformComponent>()->acceleration = {0, accel};
        }
    }
}

void Assignment03::processMessages()
{
    // receive messages
    for (auto const& msg : mMessages)
    {
        switch (msg.type)
        {
        case MessageType::Collision:
        {
            // not needed in this task
        }
        break;

        case MessageType::RegionDetection:
        {
            auto detector = msg.sender->entity->getComponent<RegionDetectorComponent>();
            if (msg.subject->entity->hasComponent<BallComponent>())
            {
                // keep score
                switch (detector->owner)
                {
                case Player::Left:
                    mParams.scoreRight++;
                    break;

                case Player::Right:
                    mParams.scoreLeft++;
                    break;
                }

                if (mParams.scenario == Scenario::Task2C)
                {
                    auto pos = msg.subject->entity->getComponent<TransformComponent>()->position;
                    glow::info() << "Ball left the field at y = " << pos.y;
                }

                // destroy ball
                destroyEntity(msg.subject->entity);

                // spawn new ball if last ball was destroyed
                if (mBallComps.empty())
                    spawnBall();

                // either way, reset multi ball cooldown
                mParams.multiBallCooldown = mParams.multiBallTime;
            }

            // Update display of score in the AntTweakBar
            TwRefreshBar(tweakbar());
        }
        break;
        }
    }
    mMessages.clear();
}

void Assignment03::update(float elapsedSeconds)
{
    updateBallSystem(elapsedSeconds);
    updateAI(elapsedSeconds);
    updateMotionSystem(elapsedSeconds);
    updateRegionDetectorSystem(elapsedSeconds); // before collision!
    updateCollisionSystem(elapsedSeconds);
    updatePaddleSystem(elapsedSeconds);
    updateGameLogic(elapsedSeconds);
    processMessages();
}

void Assignment03::render(float elapsedSeconds)
{
    // clear the screen in RWTH blue
    glClearColor(0.00f, 0.00f, 0.00f, 1.00f);
    glClear(GL_COLOR_BUFFER_BIT);

    // draw all render components
    {
        auto shader = mShaderObj->use();
        auto quad = mQuad->bind();

        // "Zoom" mode for aspect ratio
        glm::vec2 fieldSize = {mParams.fieldWidth, mParams.fieldHeight};
        glm::vec2 offset = {0, 0};
        glm::vec2 scale = {0, 0};
        if (getWindowWidth() > getWindowHeight())
        {
            offset.x = (1.0f - getWindowHeight() / (float)getWindowWidth()) / 2.0f;
            scale = glm::vec2(getWindowHeight() / (float)getWindowWidth(), 1.0f) / fieldSize;
        }
        else
        {
            offset.y = (1.0f - getWindowWidth() / (float)getWindowHeight()) / 2.0f;
            scale = glm::vec2(1.0f, getWindowWidth() / (float)getWindowHeight()) / fieldSize;
        }

        // blue area
        shader.setUniform("uPosition", offset);
        shader.setUniform("uSize", fieldSize * scale);
        shader.setUniform("uSphere", false);
        shader.setUniform("uColor", glm::vec3(0.00f, 0.33f, 0.62f));
        quad.draw();

        // render entities
        for (auto const& renderComp : mRenderComps)
        {
            auto transformComp = renderComp->entity->getComponent<TransformComponent>();
            auto shapeComp = renderComp->entity->getComponent<ShapeComponent>();
            auto boxShape = dynamic_cast<BoxShapeComponent*>(shapeComp);
            auto sphereShape = dynamic_cast<SphereShapeComponent*>(shapeComp);

            glm::vec2 halfSize;

            if (boxShape)
            {
                halfSize = boxShape->halfExtent;
                shader.setUniform("uSphere", false);
            }
            else if (sphereShape)
            {
                halfSize = glm::vec2(sphereShape->radius);
                shader.setUniform("uSphere", true);
            }
            else
                glow::error() << "Shape not supported";

            shader.setUniform("uSize", 2 * halfSize * scale);
            shader.setUniform("uPosition", (transformComp->position - halfSize) * scale + offset);

            shader.setUniform("uColor", renderComp->color);

            quad.draw();
        }
    }
}

void Assignment03::sendMessage(const Message& msg)
{
    mMessages.push_back(msg);
}

void Assignment03::destroyEntity(Entity* entity)
{
    SharedEntity se;
    for (auto const& e : mEntities)
        if (e.get() == entity)
        {
            se = e;
            break;
        }

    removeElement(mEntities, se); // se is pinned until function exits, thus safe to use
    for (auto c : se->getComponents())
    {
        removeElement(mTransformComps, c);
        removeElement(mShapeComps, c);
        removeElement(mCollisionComps, c);
        removeElement(mRegionDetectorComps, c);
        removeElement(mRenderComps, c);
        removeElement(mBallComps, c);
        removeElement(mPaddleComps, c);
        removeElement(mAIComps, c);
    }
}

namespace
{
void TW_CALL TaskSetter(const void* value, void* clientData)
{
    auto a = (Assignment03*)clientData;
    a->setScenario((Scenario) * (int*)value);
}
void TW_CALL TaskGetter(void* value, void* clientData)
{
    *(int*)value = (int)((Assignment03*)clientData)->getParams().scenario;
}
void TW_CALL BallGetter(void* value, void* clientData)
{
    *(int*)value = ((std::vector<BallComponent>*)clientData)->size();
}
}

void Assignment03::init()
{
    // smoother rendering
    setVSync(true);

    // load simulation resources
    mQuad = geometry::Quad<>().generate();
    mShaderObj = Program::createFromFile(util::pathOf(__FILE__) + "/shaderObj");

    // setup tweakbar (we just use it as a scoreboard here)
    TwAddVarRO(tweakbar(), "Score Left", TW_TYPE_INT32, &mParams.scoreLeft, "");
    TwAddVarRO(tweakbar(), "Score Right", TW_TYPE_INT32, &mParams.scoreRight, "");

    TwEnumVal tasksEV[] = {
        {(int)Scenario::Task2A, "Task 2.a"}, //
        {(int)Scenario::Task2B, "Task 2.b"}, //
        {(int)Scenario::Task2C, "Task 2.c"}, //
        {(int)Scenario::Task3, "Task 3"},    //
    };
    TwType tasksType = TwDefineEnum("Task", tasksEV, 4);
    TwAddVarCB(tweakbar(), "Task", tasksType, TaskSetter, TaskGetter, this, "");

    TwEnumVal enemyEV[] = {
        {(int)EnemyAI::Simple, "Simple"}, //
        {(int)EnemyAI::Normal, "Normal"}, //
        {(int)EnemyAI::Good, "Good"},     //
    };
    TwType enemyType = TwDefineEnum("Enemy", enemyEV, 3);
    TwAddVarRW(tweakbar(), "Enemy AI", enemyType, &mParams.enemy, "");
    TwAddVarCB(tweakbar(), "Balls", TW_TYPE_INT32, nullptr, BallGetter, &mBallComps, "");

    TwDefine("Tweakbar size='200 120' valueswidth=80");

    // create initial entities / setup game area
    setScenario(mParams.scenario);
}

static float random(float min, float max)
{
    return min + (max - min) * (rand() / (float)RAND_MAX);
}
