///
/// Create the meshes for this chunk. (one per material)
/// The return value is a map from material to mesh.
///
/// Your job is to:
///     - enhance the performance by (re-)creating meshes only if needed
///       (Dirty-flagging can be done using mIsDirty)
///     - create faces for all visible blocks
///     - adapt Vertices.hh (vertex type) and terrain.vsh (vertex shader)
///
///     - Advanced: do not create faces that are never visible from the outside
///                 - no face between two solids
///                 - no face between two translucent materials of the same type
///     - Advanced: compute some fake ambient occlusion for every vertex
///                 (also optimize the triangulation of the quads depending on the ao values)
///     - Advanced: Pack vertex data in 16 byte or less (e.g. a single (glm::)vec4)
///
///
/// Notes:
///     - pre-filled code is meant as a starting helper; you may change everything
///       you want inside the strips, e.g. implement helper functions (see Chunk.hh)
///     - for every material (except air) in a chunk, one mesh must be created
///     - it is up to you whether you create an indexed mesh or not
///     - local coordinates: 0..size-1            -> block query: block(localPos)
///     - global coordinates: chunkPos + localPos -> block query: queryBlock(globalPos)
///     - read Chunk.hh for helpers and explanations
///     - for AO see screenshots, "Fake Ambient Occlusion.jpg", and "Fake AO - Triangulation.jpg"
///     - don't forget that some faces need information from neighboring chunks (global queries)
///
/// ============= STUDENT CODE BEGIN =============

std::map<int, SharedVertexArray> Chunk::queryMeshes()
{
    if (!isDirty())
        return mMeshes;

    GLOW_ACTION(); // time this method (shown on shutdown)

    // clear list of cached meshes
    mMeshes.clear();
    std::set<int> built; // track already built materials

    // ensure that each material is accounted for
    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                glm::ivec3 p = {x, y, z};
                auto const &b = block(p); // get block

                // if block material is not air and not already built
                if (!b.isAir() && !built.count(b.mat))
                {
                    // mark as built
                    built.insert(b.mat);

                    // create VAO
                    auto vao = buildMeshFor(b.mat);
                    if (vao) // might be nullptr if fully surrounded
                        mMeshes[b.mat] = vao;
                }
            }

    mIsDirty = false;
    glow::info() << "Rebuilding mesh for " << chunkPos;
    return mMeshes;
}

SharedVertexArray Chunk::buildMeshFor(int mat) const
{
    GLOW_ACTION(); // time this method (shown on shutdown)

    // assemble data
    std::vector<TerrainVertex> vertices;

    // optimized packed vertex
    auto vert = [](glm::vec3 pos, int dir, float ao) {
        TerrainVertex v;
        v.pos = glm::vec4(pos, ao * 0.5f + dir);
        return v;
    };

    for (auto z = 0; z < size; ++z)
        for (auto y = 0; y < size; ++y)
            for (auto x = 0; x < size; ++x)
            {
                glm::ivec3 p = {x, y, z}; // local position
                auto gp = chunkPos + p;   // global position
                auto const &blk = block(p);

                if (blk.mat != mat)
                    continue; // consider only current material

                for (auto s : {-1, 1})
                    for (auto dir : {0, 1, 2})
                    {
                        // face normal
                        auto n = s * glm::ivec3(dir == 0, dir == 1, dir == 2);

                        // neighbor position and block
                        auto np = p + n;
                        auto nblk = queryBlock(chunkPos + np); // global query

                        // check if we have to gen a face
                        bool hasFace = !nblk.isSolid() && nblk.mat != mat;

                        if (hasFace)
                        {
                            auto dn = glm::vec3(n);
                            auto dt = cross(dn, glm::vec3(dir == 1, dir == 2, dir == 0));
                            auto db = cross(dt, dn);

                            auto pc = glm::vec3(gp) + 0.5f + dn * 0.5f;

                            auto pdir = dir + (s + 1) / 2 * 3; // packed dir 0,1,2 negative, 3,4,5 positive

                            // Calculate position
                            auto p00 = pc - dt * 0.5f - db * 0.5f;
                            auto p01 = pc - dt * 0.5f + db * 0.5f;
                            auto p10 = pc + dt * 0.5f - db * 0.5f;
                            auto p11 = pc + dt * 0.5f + db * 0.5f;

                            // Ambient Occlusion trick
                            auto a00 = aoAt(gp + n, -glm::ivec3(dt), -glm::ivec3(db));
                            auto a01 = aoAt(gp + n, -glm::ivec3(dt), +glm::ivec3(db));
                            auto a10 = aoAt(gp + n, +glm::ivec3(dt), -glm::ivec3(db));
                            auto a11 = aoAt(gp + n, +glm::ivec3(dt), +glm::ivec3(db));

                            // Create face
                            // (choose optimal triangulation based on ambient occlusion)
                            if (glm::abs(a01 - a10) > glm::abs(a00 - a11))
                            {
                                vertices.push_back(vert(p00, pdir, a00));
                                vertices.push_back(vert(p01, pdir, a01));
                                vertices.push_back(vert(p11, pdir, a11));

                                vertices.push_back(vert(p00, pdir, a00));
                                vertices.push_back(vert(p11, pdir, a11));
                                vertices.push_back(vert(p10, pdir, a10));
                            }
                            else
                            {
                                vertices.push_back(vert(p00, pdir, a00));
                                vertices.push_back(vert(p01, pdir, a01));
                                vertices.push_back(vert(p10, pdir, a10));

                                vertices.push_back(vert(p01, pdir, a01));
                                vertices.push_back(vert(p11, pdir, a11));
                                vertices.push_back(vert(p10, pdir, a10));
                            }
                        }
                    }
            }

    if (vertices.empty())
        return nullptr; // no visible faces

    auto ab = ArrayBuffer::create(TerrainVertex::attributes());
    ab->bind().setData(vertices);

    glow::info() << "Created " << vertices.size() << " verts for mat " << mat << " in chunk " << chunkPos;
    return VertexArray::create(ab);
}

float Chunk::aoAt(glm::ivec3 pos, glm::ivec3 dx, glm::ivec3 dy) const
{
    // block(pos) is non-solid

    // query three relevant materials
    auto s10 = queryBlock(pos + dx).isSolid();
    auto s01 = queryBlock(pos + dy).isSolid();
    auto s11 = queryBlock(pos + dx + dy).isSolid();

    if (s10 && s01)
        s11 = true; // corner case

    return (3 - s10 - s01 - s11) / 3.0f;
}

/// ============= STUDENT CODE END =============