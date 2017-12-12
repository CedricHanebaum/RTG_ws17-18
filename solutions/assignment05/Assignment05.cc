/// ===============================================================================================
/// Task 1.a
/// Build the line mesh
/// Draw the line mesh
///
/// Your job is to:
///     - build a line mesh for one single line
///
/// Notes:
///     - store the VertexArray to the member variable mMeshLine
///     - the primitive type is GL_LINES
///     - for the drawing, you have to set some uniforms for mShaderLine
///     - uViewMatrix and uProjectionMatrix are automatically set
///
///
/// ============= STUDENT CODE BEGIN =============
void Assignment05::buildLineMesh()
{
    // create array buffer with two float vertices: 0.0 and 1.0
    auto ab = ArrayBuffer::create();
    ab->defineAttribute<float>("aPosition");
    ab->bind().setData(std::vector<float>({0.0f, 1.0f}));
    mMeshLine = VertexArray::create(ab, GL_LINES);
}

void Assignment05::drawLine(glm::vec3 from, glm::vec3 to, glm::vec3 color)
{
    // pass start and end via uniforms
    auto shader = mShaderLine->use();
    shader.setUniform("uFrom", from);
    shader.setUniform("uTo", to);
    shader.setUniform("uColor", color);

    mMeshLine->bind().draw();
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================



/// ===============================================================================================
/// Task 1.b
/// Draw the rigid body
///
/// Your job is to:
///     - compute the model matrix of the box or sphere
///     - pass it to the shader as uniform
///
/// Notes:
///     - a rigid body can consist of several shapes with one common
///       transformation matrix (rbTransform)
///     - furthermore, every shape is transformed by its own matrix (s->transform)
///       and some scaling that is defined by the radius (sphere) or halfExtent (box)
///
/// ============= STUDENT CODE BEGIN =============
auto rbTransform = mRigidBody.getTransform();

for (auto const& s : mRigidBody.shapes)
{
    auto sphere = std::dynamic_pointer_cast<SphereShape>(s);
    auto box = std::dynamic_pointer_cast<BoxShape>(s);

    // draw spheres
    if (sphere)
    {
        auto model = s->transform * glm::scale(glm::vec3(sphere->radius));
        shader.setUniform("uModelMatrix", rbTransform * model);
        mMeshSphere->bind().draw();
    }

    // draw boxes
    if (box)
    {
        auto model = s->transform * glm::scale(box->halfExtent);
        shader.setUniform("uModelMatrix", rbTransform * model);
        mMeshCube->bind().draw();
    }
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================