#include "RigidBody.hh"

#include <glow/common/log.hh>

#include <glm/ext.hpp>

void RigidBody::calculateMassAndInertia()
{
    // resolution for sampling
    auto const targetVolume = 0.001f;

    // create samples
    std::vector<ShapeSample> samples;
    for (auto const& shape : shapes)
        shape->generateSamples(samples, targetVolume);

    // compute linear and rotational mass via sampling
    mass = 0.0f;
    glm::vec3 cog; // center of gravity
    inertia = glm::mat3(0.0f);

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

	for (auto const& s : samples)
	{
		// s.position (sample position in body space)
		// s.mass     (sample mass)
        mass += s.mass;
        cog += s.mass * s.position; // maybe convert position to world space here?
	}

    cog *= (1 / mass);

    for(auto const& s: samples) {
        auto r = s.position - cog;
        // column major order
        inertia += s.mass * glm::mat3(glm::vec3(std::pow(r.y, 2) + std::pow(r.z, 2), -r.y * r.x, -r.z * r.x),
                                      glm::vec3(-r.x * r.y, std::pow(r.z, 2) + std::pow(r.x, 2), -r.z * r.y),
                                      glm::vec3(-r.x * r.z, -r.y * r.z, std::pow(r.x, 2) + std::pow(r.y, 2)));
    }

/// ============= STUDENT CODE END =============

    // move to center of mass
    moveShapes(-cog);
    linearPosition += cog;
    glow::info() << "Calculated mass and inertia via " << samples.size() << " samples.";
    glow::info() << " - Mass is " << mass;
    glow::info() << " - Center of Gravity is " << cog;
    glow::info() << " - Inertia is " << cog;
    for (auto i = 0; i < 3; ++i)
        glow::info() << "    " << inertia[i];
}

void RigidBody::loadPreset(Preset preset)
{
    shapes.clear();
    this->preset = preset;

    linearPosition = glm::vec3(0.0);
    linearMomentum = glm::vec3(0.0);

    angularPosition = glm::mat3();
    angularMomentum = glm::vec3(0.0);

    switch (preset)
    {
    case Preset::Pendulum:
        // Would be better to use a point constraint for the particle
        shapes.push_back(std::make_shared<BoxShape>(glm::vec3(3, 1.5f, 1), 10.0f, glm::mat4()));
        linearPosition = {0, 3, 0};
        break;

    case Preset::RolyPolyToy:
        // head
        shapes.push_back(std::make_shared<SphereShape>(2.0f, 3.0f, glm::mat4()));
        // heavy weight inside the bottom sphere (should be lower than bottom sphere center)
        shapes.push_back(std::make_shared<SphereShape>(0.3f, 15.0f, glm::translate(glm::vec3(0, -1.5f, 0))));
        // bottom sphere
        shapes.push_back(std::make_shared<SphereShape>(1.3f, 2.0f, glm::translate(glm::vec3(0, 2.5f, 0))));
        // left eye (as seen from front)
        shapes.push_back(std::make_shared<SphereShape>(0.3f, 0.1f, glm::translate(glm::vec3(-0.5f, 3.0f, 1.0f))));
        // right eye (as seen from front)
        shapes.push_back(std::make_shared<SphereShape>(0.3f, 0.1f, glm::translate(glm::vec3(0.5f, 3.0f, 1.0f))));
        // right arm (as seen from front)
        shapes.push_back(std::make_shared<BoxShape>(
            glm::vec3(0.3f, 0.7f, 0.3f), 1.0f,
            glm::translate(glm::vec3(-1.5f, 2.0f, 0)) * glm::rotate(glm::radians(45.0f), glm::vec3(0, 0, 1))));
        // left arm (as seen from front)
        shapes.push_back(std::make_shared<BoxShape>(
            glm::vec3(0.3f, 0.7f, 0.3f), 1.0f,
            glm::translate(glm::vec3(1.5f, 2.0f, 0)) * glm::rotate(glm::radians(-45.0f), glm::vec3(0, 0, 1))));


        linearPosition = {0, 2, 0};
        break;
    }
}

void RigidBody::update(float elapsedSeconds)
{
    auto pendulumDistance = distance(linearPosition, pendulumPosition);

    auto omega = angularVelocity();

    // clamp if rotating too fast
    auto const maxOmega = 100.0f;
    if (length(omega) > maxOmega)
    {
        omega = normalize(omega) * maxOmega;
        glow::warning() << "Angular velocity too high, clamping! (should not happen in correct solutions)";
    }

    // motion equations
    {
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

        // Debug
        omega = glm::vec3(0, 1, 0);

		// update
		// - linearMomentum
        linearMomentum = linearForces * elapsedSeconds;
		// - linearPosition
        auto v = (1 / mass) * linearMomentum;
        linearPosition += v * elapsedSeconds;
		// - angularMomentum
        angularMomentum = angularForces * elapsedSeconds;
		// - angularPosition

        // Hier stimmt irgendwas nicht. angularPosition[1] ist zu beginn (0, 1, 0)^T.
        // Nach dem Kreuzprodukt kommt dann (0, 0, 0)^T raus. Unten beim normalisieren wird dann
        // daraus NaN. Darum (wahrscheinlich) wird nichts angezeigt.
        glm::vec3 r1 = glm::cross(omega, glm::column(angularPosition, 0)) * elapsedSeconds;
        glm::vec3 r2 = glm::cross(omega, glm::column(angularPosition, 1)) * elapsedSeconds;
        glm::vec3 r3 = glm::cross(omega, glm::column(angularPosition, 2)) * elapsedSeconds;
        angularPosition = glm::mat3(r1, r2, r3);


/// ============= STUDENT CODE END =============

        // re-orthogonalize rotation matrix
        angularPosition[1] -= angularPosition[0] * dot(angularPosition[0], angularPosition[1]);
        angularPosition[2] -= angularPosition[0] * dot(angularPosition[0], angularPosition[2]);
        angularPosition[2] -= angularPosition[1] * dot(angularPosition[1], angularPosition[2]);

        angularPosition[0] = normalize(angularPosition[0]);
        angularPosition[1] = normalize(angularPosition[1]);
        angularPosition[2] = normalize(angularPosition[2]);
    }

    // damping
    linearMomentum *= glm::pow(1.0f - linearDamping, elapsedSeconds);
    angularMomentum *= glm::pow(1.0f - angularDamping, elapsedSeconds);

    // apply pendulum constraint
    if (preset == Preset::Pendulum)
    {
        auto dir = normalize(linearPosition - pendulumPosition);

        linearMomentum -= dot(dir, linearMomentum) * dir;
        linearPosition = pendulumPosition + dir * pendulumDistance;
    }
}

void RigidBody::checkPlaneCollision(glm::vec3 planePos, glm::vec3 planeNormal)
{
    // collide shapes
    for (auto const& s : shapes)
        s->checkPlaneCollision(*this, planePos, planeNormal);
}

void RigidBody::computeCollision(glm::vec3 worldPos, glm::vec3 otherNormal)
{
    auto relPos = worldPos - linearPosition;

    // get local velocity
    auto velo = velocityInPoint(worldPos);

    // split into tangent and normal dir
    auto vNormal = otherNormal * dot(otherNormal, velo);

    if (dot(otherNormal, vNormal) > 0)
        return; // already separating

    // see http://www.cs.cmu.edu/~baraff/sigcourse/notesd2.pdf p.18
    auto vrel = vNormal;
    auto num = -(1 + restitution) * vrel;
    auto t1 = 1.0f / mass;
    auto t2 = 0.0f; // inf mass
    auto t3 = dot(otherNormal, cross(invInertiaWorld() * cross(relPos, otherNormal), relPos));
    auto t4 = 0.0f; // inf inertia

    auto j = num / (t1 + t2 + t3 + t4);
    auto impulse = otherNormal * j;

    // apply correcting impulse in normal dir
    applyImpulse(impulse, worldPos);
}

void RigidBody::moveShapes(glm::vec3 offset)
{
    for (auto const& s : shapes)
        s->transform = glm::translate(offset) * s->transform;
}

glm::vec3 RigidBody::pointLocalToGlobal(glm::vec3 localPos) const
{
    return linearPosition + angularPosition * localPos;
}

glm::vec3 RigidBody::pointGlobalToLocal(glm::vec3 worldPos) const
{
    return transpose(angularPosition) * (worldPos - linearPosition);
}

glm::vec3 RigidBody::directionLocalToGlobal(glm::vec3 localDir) const
{
    return angularPosition * localDir;
}

glm::vec3 RigidBody::directionGlobalToLocal(glm::vec3 worldDir) const
{
    return transpose(angularPosition) * worldDir;
}

glm::mat4 RigidBody::getTransform() const
{
    return glm::translate(linearPosition) * glm::mat4(angularPosition);
}

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
	return glm::vec3(0.0f); // TODO
}

glm::vec3 RigidBody::angularVelocity() const
{
	return glm::vec3(0.0f); // TODO
}

glm::vec3 RigidBody::velocityInPoint(glm::vec3 worldPos) const
{
	return glm::vec3(0.0f); // TODO
}

glm::mat3 RigidBody::invInertiaWorld() const
{
	return glm::mat3(0.0f); // TODO
}

void RigidBody::applyImpulse(glm::vec3 impulse, glm::vec3 worldPos)
{
	// TODO
}

void RigidBody::addForce(glm::vec3 force, glm::vec3 worldPos)
{
	// TODO
}

/// ============= STUDENT CODE END =============

void RigidBody::clearForces()
{
    linearForces = glm::vec3();
    angularForces = glm::vec3();

    // gravity
    linearForces += glm::vec3(0, gravity, 0) * mass;
}

bool RigidBody::rayCast(glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3& hitPos, glm::vec3& hitNormal, float maxRange) const
{
    hitPos = rayPos + rayDir * maxRange;
    auto hasHit = false;
    auto transform = getTransform();

    for (auto const& s : shapes)
    {
        glm::vec3 pos, normal;

        if (s->rayCast(transform, rayPos, rayDir, pos, normal, maxRange))
        {
            if (distance2(rayPos, pos) < distance2(rayPos, hitPos))
            {
                hitPos = pos;
                hitNormal = normal;
                hasHit = true;
            }
        }
    }

    return hasHit;
}
