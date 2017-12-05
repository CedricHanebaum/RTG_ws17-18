#include "Shapes.hh"

#include "RigidBody.hh"

#include <glow/common/log.hh>

#include <random>

// returns a uniform float in [min, max]
static float random(float min, float max);
// returns a uniform vec3 in the unit sphere
static glm::vec3 randomInUnitSphere();

void SphereShape::generateSamples(std::vector<ShapeSample> &samples, float targetVolume) const
{
    auto volume = radius * radius * radius * 4.0f / 3.0f * glm::pi<float>();
    auto n = (int)glm::max(1.0f, glm::round(volume / targetVolume));

    for (auto i = 0; i < n; ++i)
    {
        auto pos = randomInUnitSphere() * radius;

        ShapeSample sample;
        sample.mass = mass / n;
        sample.position = glm::vec3(transform * glm::vec4(pos, 1.0f));
        samples.push_back(sample);
    }
}

bool SphereShape::rayCast(glm::mat4 const &rbTransform, glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3 &hitPos, glm::vec3 &hitNormal, float maxRange) const
{
    // center in world space
    auto worldCenter = glm::vec3(rbTransform * transform * glm::vec4(0, 0, 0, 1));
    auto d = worldCenter - rayPos;
    auto closestP = rayPos + rayDir * dot(d, rayDir);
    auto dis = closestP - worldCenter;

    auto r = length(dis);
    if (r < radius)
    {
        auto delta = glm::sqrt(radius * radius - r * r);
        if (delta > distance(closestP, rayPos))
            delta *= -1.0f; // inside sphere

        hitPos = closestP - rayDir * delta;
        hitNormal = normalize(hitPos - worldCenter);
        return true;
    }

    return false;
}

void SphereShape::checkPlaneCollision(RigidBody &body, glm::vec3 planePos, glm::vec3 planeNormal) const
{
    // check if hit
    auto m = body.getTransform() * transform;
    auto worldCenter = glm::vec3(m * glm::vec4(0, 0, 0, 1));
    auto dis = dot(worldCenter - planePos, planeNormal);
    if (dis < radius) // collision
    {
        // handle collision
        auto planeHit = worldCenter - planeNormal * dis;
        body.computeCollision(planeHit, planeNormal);

        // fix position
        body.linearPosition += planeNormal * (radius - dis);
    }
}

void BoxShape::generateSamples(std::vector<ShapeSample> &samples, float targetVolume) const
{
    auto volume = halfExtent.x * halfExtent.y * halfExtent.z * 8.0f;
    auto n = (int)glm::max(1.0f, glm::round(volume / targetVolume));

    for (auto i = 0; i < n; ++i)
    {
        auto pos = glm::vec3(random(-halfExtent.x, halfExtent.x), //
                             random(-halfExtent.y, halfExtent.y), //
                             random(-halfExtent.z, halfExtent.z)  //
        );

        ShapeSample sample;
        sample.mass = mass / n;
        sample.position = glm::vec3(transform * glm::vec4(pos, 1.0f));
        samples.push_back(sample);
    }
}

bool BoxShape::rayCast(glm::mat4 const &rbTransform, glm::vec3 rayPos, glm::vec3 rayDir, glm::vec3 &hitPos, glm::vec3 &hitNormal, float maxRange) const
{
    auto m4 = rbTransform * transform;
    auto m3 = glm::mat3(m4);

    hitPos = rayPos + rayDir * maxRange;
    auto hasHit = false;

    for (auto s : {-1.0f, 1.0f})
        for (auto d : {0, 1, 2})
        {
            auto normal = glm::vec3(d == 0, d == 1, d == 2);
            auto tangent0 = glm::vec3(d == 1, d == 2, d == 0);
            auto tangent1 = glm::vec3(d == 2, d == 0, d == 1);

            auto c = normal * dot(normal, halfExtent) * s;
            auto d0 = dot(tangent0, halfExtent);
            auto d1 = dot(tangent1, halfExtent);

            auto worldNormal = m3 * normal * s;
            auto worldTangent0 = m3 * tangent0 * s;
            auto worldTangent1 = m3 * tangent1 * s;
            auto worldCenter = glm::vec3(m4 * glm::vec4(c, 1));

            if (glm::abs(dot(worldNormal, rayDir)) < 0.0001)
                continue; // near parallel

            // project to plane
            auto toRayPos = rayPos - worldCenter;
            auto planePos = rayPos - rayDir * dot(toRayPos, worldNormal) / dot(rayDir, worldNormal);

            // check if inside
            auto planeDir = planePos - worldCenter;
            if (glm::abs(dot(worldTangent0, planeDir)) < d0 && glm::abs(dot(worldTangent1, planeDir)) < d1)
            {
                // TODO: handle inside-box scenario

                // check if closer
                if (distance2(rayPos, planePos) < distance2(rayPos, hitPos))
                {
                    hitPos = planePos;
                    hitNormal = worldNormal;
                    hasHit = true;
                }
            }
        }

    return hasHit;
}

void BoxShape::checkPlaneCollision(RigidBody &body, glm::vec3 planePos, glm::vec3 planeNormal) const
{
    auto m = body.getTransform() * transform;

    for (auto dx : {-1, 1})
        for (auto dy : {-1, 1})
            for (auto dz : {-1, 1})
            {
                auto localPos = halfExtent * glm::vec3(dx, dy, dz);
                auto worldPos = glm::vec3(m * glm::vec4(localPos, 1.0f));

                auto dis = dot(worldPos - planePos, planeNormal);
                if (dis < 0)
                {
                    // handle collision
                    body.computeCollision(worldPos, planeNormal);

                    // fix position
                    body.linearPosition += planeNormal * (-dis);
                }
            }
}

static std::random_device sRandDevice;

static glm::vec3 randomInUnitSphere()
{
    float x, y, z;

    do
    {
        x = random(-1.0f, 1.0f);
        y = random(-1.0f, 1.0f);
        z = random(-1.0f, 1.0f);
    } while (x * x + y * y + z * z > 1.0f);

    return {x, y, z};
}
static float random(float min, float max)
{
    return std::uniform_real_distribution<float>(min, max)(sRandDevice);
}

Shape::~Shape() = default;
