#pragma once

#include <vector>

#include <glm/ext.hpp>

struct ShapeSample
{
    float mass;
    glm::vec3 position;
};

struct RigidBody;

struct Shape
{
    float mass;
    glm::mat4 transform;

    Shape(float mass, glm::mat4 transform) : mass(mass), transform(transform) {}

    virtual ~Shape(); // otherwse undefined behavior

    virtual void generateSamples(std::vector<ShapeSample>& samples, float targetVolume) const = 0;

    virtual bool rayCast(glm::mat4 const& rbTransform,
                         glm::vec3 rayPos,
                         glm::vec3 rayDir,
                         glm::vec3& hitPos,
                         glm::vec3& hitNormal,
                         float maxRange) const = 0;

    virtual void checkPlaneCollision(RigidBody& body, glm::vec3 planePos, glm::vec3 planeNormal) const = 0;
};

struct SphereShape : Shape
{
    float const radius;

    SphereShape(float r, float mass, glm::mat4 transform) : Shape(mass, transform), radius(r) {}

    void generateSamples(std::vector<ShapeSample>& samples, float targetVolume) const override;

    bool rayCast(glm::mat4 const& rbTransform, glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3& hitPos, glm::vec3& hitNormal, float maxRange) const override;

    void checkPlaneCollision(RigidBody& body, glm::vec3 planePos, glm::vec3 planeNormal) const override;
};

struct BoxShape : Shape
{
    glm::vec3 const halfExtent;

    BoxShape(glm::vec3 halfExtent, float mass, glm::mat4 transform) : Shape(mass, transform), halfExtent(halfExtent) {}

    void generateSamples(std::vector<ShapeSample>& samples, float targetVolume) const override;

    bool rayCast(glm::mat4 const& rbTransform, glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3& hitPos, glm::vec3& hitNormal, float maxRange) const override;

    void checkPlaneCollision(RigidBody& body, glm::vec3 planePos, glm::vec3 planeNormal) const override;
};
