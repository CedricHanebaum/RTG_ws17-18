/// ===============================================================================================
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
auto n0 = normalize(cross(p00 - p10, p01 - p10));
auto n1 = normalize(cross(p10 - p11, p01 - p11));

vertices.push_back({p00, n0, c00});
vertices.push_back({p01, n0, c01});
vertices.push_back({p10, n0, c10});

vertices.push_back({p01, n1, c01});
vertices.push_back({p11, n1, c11});
vertices.push_back({p10, n1, c10});
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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
        glm::vec3 n;
        auto p = pos(x, y);

        if (x > 0 && y > 0)
            n += cross(pos(x, y - 1) - p, pos(x - 1, y) - p);

        if (x > 0 && y < res - 1)
            n += cross(pos(x - 1, y) - p, pos(x, y + 1) - p);

        if (x < res - 1 && y < res - 1)
            n += cross(pos(x, y + 1) - p, pos(x + 1, y) - p);

        if (x < res - 1 && y > 0)
            n += cross(pos(x + 1, y) - p, pos(x, y - 1) - p);

        normals[y * res + x] = normalize(n);
    }
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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
auto pc = (p00 + p01 + p10 + p11) / 4.0f;
auto cc = (c00 + c01 + c10 + c11) / 4.0f;

auto n00 = normals[(y + 0) * res + (x + 0)];
auto n01 = normals[(y + 1) * res + (x + 0)];
auto n10 = normals[(y + 0) * res + (x + 1)];
auto n11 = normals[(y + 1) * res + (x + 1)];
auto nc = normalize(n00 + n01 + n10 + n11);

vertices.push_back({p00, n00, c00});
vertices.push_back({pc, nc, cc});
vertices.push_back({p10, n10, c10});

vertices.push_back({p10, n10, c10});
vertices.push_back({pc, nc, cc});
vertices.push_back({p11, n11, c11});

vertices.push_back({p11, n11, c11});
vertices.push_back({pc, nc, cc});
vertices.push_back({p01, n01, c01});

vertices.push_back({p01, n01, c01});
vertices.push_back({pc, nc, cc});
vertices.push_back({p00, n00, c00});
/// ============= STUDENT CODE END =============
/// ===============================================================================================

        

/// ===============================================================================================
/// Task 2.a
/// Particles move according to forces that act upon them.
///
/// Your job is to:
///     - update the particle velocity (p.velocity)
///     - update the particle position (p.position)
///
/// Notes:
///     - particle force is stored in p.accumulatedForces
///     - particle mass is stored in p.mass
///
/// ============= STUDENT CODE BEGIN =============
auto acc = p.accumulatedForces / p.mass;

p.velocity += elapsedSeconds * acc;
p.position += elapsedSeconds * p.velocity;
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
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
glm::vec3 diff = s.p0->position - s.p1->position;
float dist = glm::length(diff);
float strain = dist - s.restDistance;

auto dir = diff / dist;

auto f = -springK * dir * strain;

// accumulate opposed forces
s.p0->accumulatedForces += f;
s.p1->accumulatedForces += -f;
/// ============= STUDENT CODE END =============
/// ===============================================================================================
        


/// ===============================================================================================
/// Task 2.c
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
/// ===============================================================================================



/// ===============================================================================================
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
for (auto& p : particles)
{
    p.accumulatedForces += glm::vec3(0, gravity, 0) * p.mass;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================
        


/// ===============================================================================================
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
auto diff = p.position - center;
auto dist = length(diff);
if (dist < radius)
{
    auto collisionNormal = diff / dist;

    // move particle to sphere surface
    p.position = center + collisionNormal * radius;

    // remove normal component of velocity
    p.velocity -= dot(p.velocity, collisionNormal) * collisionNormal;
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================