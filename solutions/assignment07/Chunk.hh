/// All Tasks
///
/// You can use this space for declarations of helper functions
///
/// ============= STUDENT CODE BEGIN =============
/// Builds the mesh for a given material
glow::SharedVertexArray buildMeshFor(int mat) const;
/// Returns the ambient occlusion at a given position
float aoAt(glm::ivec3 pos, glm::ivec3 dx, glm::ivec3 dy) const;
/// ============= STUDENT CODE END =============