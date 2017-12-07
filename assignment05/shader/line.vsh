uniform mat4 uViewMatrix;
uniform mat4 uProjectionMatrix;

/// Task 1.a
/// Draw the line mesh
///
/// Your job is to:
///     - define the vertex attributes and uniforms that you need for the rendering
///     - compute the position, transform it to screen space and pass it on
///
/// Notes:
///     - gl_Position is four-dimensional
///
/// ============= STUDENT CODE BEGIN =============

// maybe some attributes?
in vec3 vPosition;

// maybe some uniforms?
uniform mat4 uModelMatrix;

void main() 
{
    vec4 worldPos = uModelMatrix * vec4(vPosition, 1.0);
    vec4 viewPos = uViewMatrix * worldPos;
    gl_Position = uProjectionMatrix * viewPos;
}

/// ============= STUDENT CODE END =============
