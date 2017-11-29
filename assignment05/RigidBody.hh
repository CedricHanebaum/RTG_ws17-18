#pragma once

#include <glow/common/shared.hh>

#include <glm/ext.hpp>

#include "Shapes.hh"

GLOW_SHARED(struct, Shape);

enum class Preset
{
    Pendulum,
    RolyPolyToy
};

/**
 * @brief A RigidBody
 *
 * Notes:
 *  - we ignore encapsulation (getter/setter) here to reduce boilerplate
 *  - relPos is a relative position in world space ("worldPos - linearPos")
 *
 */
struct RigidBody
{
public: // fields
    // preset
    Preset preset;

    // geometry
    std::vector<SharedShape> shapes;
    glm::vec3 pendulumPosition = {0, 10, 0};

    // motion state
    // NOTE: optimized implementations would store invMass and invInertia
    float mass = 0.0f;
    glm::mat3 inertia;

    glm::vec3 linearPosition;  // translation
    glm::mat3 angularPosition; // rotation

    glm::vec3 linearMomentum;  // mass * velocity
    glm::vec3 angularMomentum; // inertia * angular velocity

    glm::vec3 linearForces;  // force
    glm::vec3 angularForces; // torque

    float linearDamping = 0.1f;
    float angularDamping = 0.1f;

    float restitution = 0.5f;
    float friction = 0.5f;

    float gravity = -9.81f;

public: // properties
    /// returns the current transformation matrix
    glm::mat4 getTransform() const;

    /// returns the linear velocity
    glm::vec3 linearVelocity() const;

    /// returns the angular velocity
    glm::vec3 angularVelocity() const;

    /// returns the world velocity at a given point
    glm::vec3 velocityInPoint(glm::vec3 worldPos) const;

    /// converts a rigid-body local point (0 is center of gravity) to a world position
    glm::vec3 pointLocalToGlobal(glm::vec3 localPos) const;

    /// converts a global position to a local one
    glm::vec3 pointGlobalToLocal(glm::vec3 worldPos) const;

    /// converts a rigid-body local direction to a world direction
    glm::vec3 directionLocalToGlobal(glm::vec3 localDir) const;

    /// converts a global direction to a local one
    glm::vec3 directionGlobalToLocal(glm::vec3 worldDir) const;

    /// recalculates world inertia
    glm::mat3 invInertiaWorld() const;

public: // const functions
    /// performas a raycast against this rigid body
    /// resulting hitPos and hitNormal are in global space
    bool rayCast(glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3& hitPos, glm::vec3& hitNormal, float maxRange = 10000.0f) const;

public: // setup functions
    /// calculates mass and inertia from shapes
    void calculateMassAndInertia();

    /// loads a given preset
    void loadPreset(Preset preset);

    /// moves all shapes
    void moveShapes(glm::vec3 offset);

public: // mutating functions
    /// applies equation of motion
    void update(float elapsedSeconds);

    /// clears linear and angular forces
    void clearForces();

    /// adds a force to a given relative position (0 is center of gravity)
    /// force itself is in world coords
    void addForce(glm::vec3 force, glm::vec3 worldPos);

    /// applies an impulse at a relative point
    /// impulse itself is in world coords
    void applyImpulse(glm::vec3 impulse, glm::vec3 worldPos);

    /// performs collision handling against a plane
    void checkPlaneCollision(glm::vec3 planePos, glm::vec3 planeNormal);

    /// applies collision handling for a given relPos and normal of other object
    /// does NOT correct position
    void computeCollision(glm::vec3 worldPos, glm::vec3 otherNormal);
};
