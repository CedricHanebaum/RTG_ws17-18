#pragma once

#include <glm/glm.hpp>
#include <glow/fwd.hh>
#include <glow/objects/ArrayBufferAttribute.hh>
#include <vector>

enum class Coloring
{
    Checker,
    Stress,
    Smooth
};

enum class FixedParticles
{
    FourSides,
    TwoSides,
    OneSide,
    Corners,
};

/// A particle is a grid point in the cloth mesh.
/// It has a certain mass and position while it can be exposed to forces
struct Particle
{
    glm::vec3 initialPosition;

    glm::vec3 position;
    float mass = 1.0f;
    bool fixed = false; // make static, e.g. for hanging up the cloth
    glm::vec3 velocity;
    glm::vec3 accumulatedForces;
    float stress = 1.0f;

    Particle() = default;
    Particle(glm::vec3 pos, float mass) : initialPosition(pos), position(pos), mass(mass) {}
};

/// A spring connects two particles. The initial particle distance is stored in restDistance
/// Whenever the actual distance is unequal to the restDistance, there is some strain going on
struct Spring
{
    float restDistance = -1.0f;
    Particle* p0 = nullptr;
    Particle* p1 = nullptr;

    Spring(Particle* particle0, Particle* particle1) : p0(particle0), p1(particle1)
    {
        restDistance = glm::distance(p0->position, p1->position);
    }
};

/// Vertex attributes for all cloth particles
struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;

    static std::vector<glow::ArrayBufferAttribute> attributes()
    {
        return {
            {&Vertex::position, "aPosition"}, //
            {&Vertex::normal, "aNormal"},     //
            {&Vertex::color, "aColor"},
        };
    }
};

class Cloth
{
public: // configurables
    float springK = 4.0f;
    float dampingD = 0.02f;
    float gravity = -9.81f;

    bool smoothShading = false;

    Coloring coloring = Coloring::Checker;

    FixedParticles fixedParticles = FixedParticles::OneSide;

private:
    /// All particles within the cloth mesh
    std::vector<Particle> particles;
    /// All particle connections
    std::vector<Spring> springs;
    /// The cloth consists of res*res particles
    int res = -1;
    /// Particle that is currently dragged by user interaction
    Particle* draggedParticle = nullptr;
    /// Current position of dragged particle
    glm::vec3 draggedPosition;
    /// Vertex array and Array Buffer for the rendering
    glow::SharedVertexArray clothVAO;
    glow::SharedArrayBuffer clothAB;

public:
    /// Cloth ctor
    /// res*res particles are created
    /// Finale cloth is <size>*<size> m^2 and weighs <weight> kg.
    Cloth(int res = 20, float size = 10.0f, float weight = 10.0f);

    /// Resets motion state the initial config and applies proper fixing
    void resetMotionState();

    /// Query a particle by its position in the reference grid
    Particle* particle(int x, int y) { return &particles[y * res + x]; }

    /// Query particle 3D position by its position in reference grid
    glm::vec3 pos(int x, int y) { return particle(x, y)->position; }

    /// Returns color for a given pos
    glm::vec3 color(int x, int y, int dx, int dy);

    /// Add a new spring that connects particles p0 and p1
    void addSpring(Particle* p0, Particle* p1) { springs.push_back({p0, p1}); }

    /// Create the actual vertex array for rendering
    glow::SharedVertexArray createVAO();

    /// Get the number of particles in one dimension
    int getResolution() const { return res; }

    /// Do the actual cloth simulation
    void updateInit(float elapsedSeconds);
    void updateForces();
    void updateMotion(float elapsedSeconds);

    /// Simulate one sphere collision step
    void addSphereCollision(glm::vec3 center, float radius);

    /// For user interaction, send raycast from pos in direction dir
    void drag(const glm::vec3& pos, const glm::vec3& dir, glm::vec3 const& camDir);

    /// When user interaction ends (e.g. mouse button release), release the fixed particle
    void releaseDrag() { draggedParticle = nullptr; }
};
