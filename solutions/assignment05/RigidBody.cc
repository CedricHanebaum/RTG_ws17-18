/// ===============================================================================================
/// Task 2.a
/// Calculate inertia and mass via sampling
///
/// Your job is to:
///     - calculate mass (write to variable 'mass')
///     - calculate center of gravity (write to variable 'cog')
///     - calculate inertia matrix (write to variable 'inertia')
///
/// Notes:
///     - sanity check for inertia: it should be roughly diagonal (sides are |.| < 0.5)
///
/// ============= STUDENT CODE BEGIN =============
// calculate center of gravity
for (auto const& s : samples)
{
    mass += s.mass;
    cog += s.position * s.mass;
}
cog /= mass;

// calculate inertia (via sampling)
for (auto const& s : samples)
{
    auto m = s.mass;
    auto r = s.position - cog;

    auto I = glm::mat3(0.0f);

    // diagonal
    I[0][0] = r.y * r.y + r.z * r.z;
    I[1][1] = r.z * r.z + r.x * r.x;
    I[2][2] = r.x * r.x + r.y * r.y;

    // sides
    I[1][0] = I[0][1] = -r.x * r.y;
    I[0][2] = I[2][0] = -r.z * r.x;
    I[2][1] = I[1][2] = -r.y * r.z;

    // add to total inertia
    inertia += I * m;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
/// Task 2.b
///
/// Your job is to:
///     - update the linear momentum and position
///     - update the angular momentum and position
///
/// Notes:
///     - you can test your angular velocity code with the following settings for omega:
///       (be sure to remove test code before uploading)
///         omega = glm::vec3(0, 1, 0) // slow counter-clockwise rotation around Y
///         omega = glm::vec3(5, 0, 0) // faster rotation around X (the long side for the pendulum)
///
///
/// ============= STUDENT CODE BEGIN =============
// linear part
linearMomentum += linearForces * elapsedSeconds;
linearPosition += linearVelocity() * elapsedSeconds;

// angular part
angularMomentum += angularForces * elapsedSeconds;

// apply rotational velocity
auto Rx = angularPosition[0];
auto Ry = angularPosition[1];
auto Rz = angularPosition[2];

Rx += cross(omega, Rx) * elapsedSeconds;
Ry += cross(omega, Ry) * elapsedSeconds;
Rz += cross(omega, Rz) * elapsedSeconds;

angularPosition = {Rx, Ry, Rz};
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
/// Task 2.c
///
/// Your job is to:
///     - implement the bodies of the six methods defined below
///
/// Notes:
///     - in some of the methods you might need to call one of the others
///     - if needed, you should use the glm functions cross, inverse and transpose
///
/// ============= STUDENT CODE BEGIN =============
glm::vec3 RigidBody::linearVelocity() const
{
    return linearMomentum / mass;
}

glm::vec3 RigidBody::angularVelocity() const
{
    return invInertiaWorld() * angularMomentum;
}

glm::vec3 RigidBody::velocityInPoint(glm::vec3 worldPos) const
{
    return linearVelocity() + cross(angularVelocity(), worldPos - linearPosition);
}

glm::mat3 RigidBody::invInertiaWorld() const
{
    return angularPosition * inverse(inertia) * transpose(angularPosition);
}

void RigidBody::applyImpulse(glm::vec3 impulse, glm::vec3 worldPos)
{
    linearMomentum += impulse;
    angularMomentum += cross(worldPos - linearPosition, impulse);
}

void RigidBody::addForce(glm::vec3 force, glm::vec3 worldPos)
{
    linearForces += force;
    angularForces += cross(worldPos - linearPosition, force);
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================