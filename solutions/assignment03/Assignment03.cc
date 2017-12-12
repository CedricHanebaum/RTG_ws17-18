/// ===============================================================================================
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
// calculate current acceleration
auto a = transformComp->acceleration;
// .. with linear drag
a -= transformComp->linearDrag * transformComp->velocity;

// apply equations of motion
transformComp->velocity += a * elapsedSeconds;
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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
// Notes:
// - tc->velocity is reflected velocity
// - can be tested in 2a
auto speed = length(tc->velocity);
auto tx = n.x;
auto ty = t * 2 - 1.0f;

auto v1 = normalize(tc->velocity);
auto v2 = normalize(glm::vec2(tx, ty));    // +- 45°
tc->velocity = normalize(v1 + v2) * speed; // rotate velocity half-ways towards shift
/// ============= STUDENT CODE END =============
/// ===============================================================================================