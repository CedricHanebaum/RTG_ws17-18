#include "Cloth.hh"

#include <glm/ext.hpp>
#include <glow/objects/ArrayBuffer.hh>
#include <glow/objects/ElementArrayBuffer.hh>
#include <glow/objects/VertexArray.hh>

using namespace glow;

Cloth::Cloth(int res, float size, float weight) : res(res)
{
    assert(res >= 2);

    auto numberOfParticles = res * res;
    particles.resize(numberOfParticles);

    // create particles
    glm::vec3 offset = size * 0.5f * glm::vec3(1, 0, 1);
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
        {
            glm::vec3 pos = {size * float(x) / res, 0, size * float(y) / res};
            pos -= offset;
            particles[y * res + x] = Particle(pos, weight / numberOfParticles);
        }

    // create springs
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
        {
            // Structural
            if (x > 0)
                addSpring(particle(x, y), particle(x - 1, y));
            if (y > 0)
                addSpring(particle(x, y), particle(x, y - 1));

            // Shearing
            if (x > 0 && y > 0)
            {
                addSpring(particle(x, y), particle(x - 1, y - 1));
                addSpring(particle(x - 1, y), particle(x, y - 1));
            }

            // Bending
            if (x > 1)
                addSpring(particle(x, y), particle(x - 2, y));
            if (y > 1)
                addSpring(particle(x, y), particle(x, y - 2));
            if (x > 1 && y > 1)
            {
                addSpring(particle(x, y), particle(x - 2, y - 2));
                addSpring(particle(x - 2, y), particle(x, y - 2));
            }
        }

    // set to proper initial motion state
    resetMotionState();
}

void Cloth::resetMotionState()
{
    for (auto& p : particles)
    {
        p.fixed = false;
        p.position = p.initialPosition;
        p.velocity = {0, 0, 0};
    }

    // fix proper particles
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
            switch (fixedParticles)
            {
            case FixedParticles::FourSides:
                if (x == 0 || y == 0 || x == res - 1 || y == res - 1)
                    particle(x, y)->fixed = true;
                break;
            case FixedParticles::TwoSides:
                if (x == 0 || x == res - 1)
                    particle(x, y)->fixed = true;
                break;
            case FixedParticles::OneSide:
                if (x == 0)
                    particle(x, y)->fixed = true;
                break;
            case FixedParticles::Corners:
                if ((x == 0 || x == res - 1) && (x + y) % (res - 1) == 0)
                    particle(x, y)->fixed = true;
                break;
            }
}

glm::vec3 Cloth::color(int x, int y, int dx, int dy)
{
    // Checker board colors
    glm::vec3 col1 = {0.00, 0.33, 0.66};
    glm::vec3 col2 = {0.66, 0.00, 0.33};

    switch (coloring)
    {
    case Coloring::Checker:
        return (x - dx + y - dy) % 2 ? col1 : col2;

    case Coloring::Stress:
        // Color dependent on strain
        return glm::mix(glm::vec3(0, 1, 0), glm::vec3(1, 0, 0), glm::min(particle(x, y)->stress * 0.03f, 1.0f));

    case Coloring::Smooth:
    default:
        return {0.00f, 0.33f, 0.66f};
    }
}

void Cloth::updateInit(float elapsedSeconds)
{
    // Reset all forces
    for (auto& p : particles)
    {
        p.stress = 0;
        p.accumulatedForces = {0, 0, 0};
    }
}

void Cloth::updateForces()
{
    /// Task 2.d
    /// Gravity force is applied to all particles
    ///
    /// Your job is to:
    ///     - add gravity force to the each particle's accumulated force
    ///
    /// Notes:
    ///     - the currently set gravity is stored in the variable gravity.
    ///     - gravity operates "downwards" (negative y-direction)
    ///     - mass is stored in the particle
    ///
    /// ============= STUDENT CODE BEGIN =============

    /// ============= STUDENT CODE END =============

    // Apply Hooke's Law
    for (auto& s : springs)
    {
        assert(s.p0 != s.p1);

        /// Task 2.b
        /// Using Hooke's law, forces act on connected particles.
        ///
        /// Your job is to:
        ///     - compare current distance and rest distance of the two particles.
        ///     - apply appropriate force to both particles
        ///
        /// Notes:
        ///     - the (stiffness) spring constant is stored in springK
        ///     - forces on the two particles counteract (i.e. contrary signs)
        ///     - if you want, you can set the particle stress variables to allow
        ///       for stress rendering (optional)
        ///
        /// ============= STUDENT CODE BEGIN =============

        auto x = glm::length(s.p0->position - s.p1->position) - s.restDistance;

        auto f0 = -springK * x * glm::normalize(s.p0->position - s.p1->position);
        auto f1 = -springK * x * glm::normalize(s.p1->position - s.p0->position);

        s.p0->accumulatedForces += f0;
        s.p1->accumulatedForces += f1;

        s.p0->stress += x * 100;
        s.p1->stress += x * 100;

        /// ============= STUDENT CODE END =============
    }

    // Dragging
    if (draggedParticle && !draggedParticle->fixed)
    {
        draggedParticle->position = draggedPosition;
        draggedParticle->velocity = {0, 0, 0};
        draggedParticle->accumulatedForces = {0, 0, 0};
    }

    // Damping
    for (auto& p : particles)
    {
        /// Task 2.d
        /// For the system to come to rest, damping is applied.
        ///
        /// your job is to:
        ///     - damp the particle forces
        ///
        /// notes:
        ///     - the damping factor is stored in dampingD
        ///     - the amount of damping also depends on particle velocity
        ///
        /// ============= STUDENT CODE BEGIN =============

        p.accumulatedForces -= dampingD * p.velocity;

        /// ============= STUDENT CODE END =============
    }
}

void Cloth::updateMotion(float elapsedSeconds)
{
    // Motion equations
    for (auto& p : particles)
    {
        if (p.fixed)
            continue;

        /// Task 2.a
        /// Particles move according to forces that act upon them.
        ///
        /// Your job is to:
        ///     - update the particle velocity (p.velocity)
        ///     - update the particle position (p.position)
        ///
        /// Notes:
        ///     - particle force is stored in p.accumulatedForce
        ///     - particle mass is stored in p.mass
        ///
        /// ============= STUDENT CODE BEGIN =============

        glm::vec3 a = p.accumulatedForces / p.mass;
        p.velocity += a * elapsedSeconds;
        p.position += p.velocity * elapsedSeconds;

        /// ============= STUDENT CODE END =============
    }
}

void Cloth::addSphereCollision(glm::vec3 center, float radius)
{
    for (auto& p : particles)
    {
        /// Task 3
        /// Check for particle-sphere collision
        ///
        /// Your job is to:
        ///     - detect whether a particle collides with the sphere
        ///     - project the particle position to the sphere surface
        ///     - project the particle velocity to the tangent plane
        ///
        /// Notes:
        ///     - The tangent plane is unambiguously defined by the normal
        ///       on the sphere surface
        ///     - After projection, the velocity vector lies in that plane
        ///
        /// ============= STUDENT CODE BEGIN =============

        /// ============= STUDENT CODE END =============
    }
}

void Cloth::drag(glm::vec3 const& pos, glm::vec3 const& dir, const glm::vec3& camDir)
{
    // if new: find particle closest to ray
    if (draggedParticle == nullptr)
    {
        auto bestDist = std::numeric_limits<float>::max();
        Particle* bestParticle = nullptr;

        for (auto& p : particles)
        {
            auto toPart = p.position - pos;
            auto closestPointOnRay = pos + dot(toPart, dir) * dir;
            auto dist = distance(p.position, closestPointOnRay);

            if (dist < bestDist)
            {
                bestDist = dist;
                bestParticle = &p;
            }
        }

        assert(bestParticle != nullptr);

        draggedParticle = bestParticle;
    }

    // set drag position
    {
        auto pPos = draggedParticle->position;
        auto toCam = pos - pPos;
        auto p = pos - dir * dot(toCam, camDir) / dot(dir, camDir);
        draggedPosition = p;
    }
}

SharedVertexArray Cloth::createVAO()
{
    std::vector<Vertex> vertices;
    vertices.reserve((3 * 2) * (res - 1) * (res - 1));

    /// Task 1.b
    ///
    ///          0,-1
    ///         / | \
    ///       /   |   \
    ///     /     |     \
    /// -1,0 --- p_c --- 1,0
    ///     \     |     /
    ///       \   |   /
    ///         \ | /
    ///          0,1
    ///
    /// Your job is to:
    ///     - calculate a smoothed normal for every particle
    ///     - the normal of p_c is the average of all adjacent triangles' normals (see ASCII pic)
    ///     - skip triangles that don't exist (border)
    ///
    /// Notes:
    ///     - particles are stored in a 2D grid with x and y from 0 to res-1 (inclusive)
    ///     - you can query the position of a particle via `pos(x, y)`
    ///     - you probably want to store them similar to the default code
    ///     - you are allowed to weight the normals by triangle area if that simplifies your code
    ///     - these triangles are only conceptual (the actual geometry is a bit different)
    ///     - notes of Task 1.a also apply
    ///
    /// ============= STUDENT CODE BEGIN =============

    std::vector<glm::vec3> normals(res * res);
    for (int y = 0; y < res; ++y)
        for (int x = 0; x < res; ++x)
        {
            glm::vec3 n1 = {0, 0, 0};
            glm::vec3 n2 = {0, 0, 0};
            glm::vec3 n3 = {0, 0, 0};
            glm::vec3 n4 = {0, 0, 0};

            if(y > 0 && x > 0)
                n1 = glm::cross(pos(x, y - 1) - pos(x, y), pos(x - 1, y) - pos(x, y));
            if(y < res - 2 && x > 0)
                n2 = glm::cross(pos(x - 1, y) - pos(x, y), pos(x, y + 1) - pos(x, y));
            if(y < res - 2 && x < res - 2)
                n3 = glm::cross(pos(x, y + 1) - pos(x, y), pos(x + 1, y) - pos(x, y));
            if(y > 0 && x < res - 2)
                n4 = glm::cross(pos(x + 1, y) - pos(x, y), pos(x, y - 1) - pos(x, y));

            glm::vec3 n = glm::normalize(glm::vec3((n1.x + n2.x + n3.x + n4.x) / 4,
                                                   (n1.y + n2.y + n3.y + n4.y) / 4,
                                                   (n1.z + n2.z + n3.z + n4.z) / 4));

            normals[y * res + x] = n;
        }

    /// ============= STUDENT CODE END =============

    // No shared vertices to allow for distinct colors.
    for (int y = 0; y < res - 1; ++y)
        for (int x = 0; x < res - 1; ++x)
        {
            auto c00 = color(x + 0, y + 0, 0, 0);
            auto c01 = color(x + 0, y + 1, 0, 1);
            auto c10 = color(x + 1, y + 0, 1, 0);
            auto c11 = color(x + 1, y + 1, 1, 1);

            auto p00 = pos(x + 0, y + 0);
            auto p01 = pos(x + 0, y + 1);
            auto p10 = pos(x + 1, y + 0);
            auto p11 = pos(x + 1, y + 1);

            if (smoothShading)
            {
                /// Task 1.c
                ///
                /// p00 -- p10
                ///  | \  / |
                ///  |  pc  |
                ///  | /  \ |
                /// p01 -- p11
                ///
                /// Your job is to:
                ///     - generate smooth-shaded geometry for the particles
                ///     - each quad should generate four triangles
                ///     - the center position is the average of the corners
                ///     - the center color is the average of the corner colors
                ///     - normals should be taken from Task 1.b
                ///     - normal for the center vertex is the average corner normal
                ///
                /// Notes:
                ///     - see notes for Task 1.a
                ///
                /// ============= STUDENT CODE BEGIN =============

                auto n00 = glm::vec3(0, 0, 0);
                auto n01 = glm::vec3(0, 0, 0);
                auto n10 = glm::vec3(0, 0, 0);
                auto n11 = glm::vec3(0, 0, 0);

                n00 = normals[y * res + x];
                if(y < res - 2)
                    n01 = normals[(y + 1) * res + x];
                if(x < res - 2)
                    n10 = normals[y * res + (x + 1)];
                if(x < res - 2 && y < res - 2)
                    n11 = normals[(y + 1) * res + (x + 1)];

                glm::vec3 pcp = glm::vec3((p00.x + p01.x + p10.x + p11.x) / 4,
                                          (p00.y + p01.y + p10.y + p11.y) / 4,
                                          (p00.z + p01.z + p10.z + p11.z) / 4);

                glm::vec3 pcc = glm::vec3((c00.x + c01.x + c10.x + c11.x) / 4,
                                          (c00.y + c01.y + c10.y + c11.y) / 4,
                                          (c00.z + c01.z + c10.z + c11.z) / 4);

                glm::vec3 pcn = glm::vec3((n00.x + n01.x + n10.x + n11.x) / 4,
                                          (n00.y + n01.y + n10.y + n11.y) / 4,
                                          (n00.z + n01.z + n10.z + n11.z) / 4);

                vertices.push_back({pcp, pcn, pcc});
                vertices.push_back({p10, n10, c10});
                vertices.push_back({p00, n00, c00});

                vertices.push_back({pcp, pcn, pcc});
                vertices.push_back({p00, n00, c00});
                vertices.push_back({p01, n01, c01});

                vertices.push_back({pcp, pcn, pcc});
                vertices.push_back({p01, n01, c01});
                vertices.push_back({p11, n11, c11});

                vertices.push_back({pcp, pcn, pcc});
                vertices.push_back({p11, n11, c11});
                vertices.push_back({p10, n10, c10});

                /// ============= STUDENT CODE END =============
            }
            else
            {
                /// Task 1.a
                ///
                /// p00 --- p10
                ///  |    /  |
                ///  |   /   |
                ///  |  /    |
                /// p01 --- p11
                ///
                /// Your job is to:
                ///     - generate flat-shaded geometry for the particles
                ///     - each quad should generate two triangles
                ///
                /// Notes:
                ///     - OpenGL default order is counter-clockwise
                ///     - you will notice inconsistent normals in the lighting
                ///     - normals must be normalized
                ///     - positions and colors are already stored in p00..p11 and c00..c11
                ///     - add vertices via `vertices.push_back({<position>, <normal>, <color>});`
                ///
                /// ============= STUDENT CODE BEGIN =============

                glm::vec3 n1 = glm::normalize(glm::cross(p10 - p00, p01 - p00));
                vertices.push_back({p00, n1, c00});
                vertices.push_back({p01, n1, c01});
                vertices.push_back({p10, n1, c10});

                glm::vec3 n2 = glm::normalize(glm::cross(p10 - p11, p01 - p11));
                vertices.push_back({p01, n2, c01});
                vertices.push_back({p10, n2, c10});
                vertices.push_back({p11, n2, c11});

                /// ============= STUDENT CODE END =============
            }
        }

    if (clothAB == nullptr)
        clothAB = ArrayBuffer::create({{&Vertex::position, "aPosition"}, //
                                       {&Vertex::normal, "aNormal"},     //
                                       {&Vertex::color, "aColor"}});

    clothAB->bind().setData(vertices);

    if (clothVAO == nullptr)
        clothVAO = VertexArray::create(clothAB);

    return clothVAO;
}
