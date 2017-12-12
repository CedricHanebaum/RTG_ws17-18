/// ===============================================================================================
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
in float aPosition; // 0.0 and 1.0

uniform vec3 uFrom;
uniform vec3 uTo;

void main() 
{
    vec3 pos = mix(uFrom, uTo, aPosition); // optimized version of "aPosition < 0.5 ? uFrom : uTo"
    gl_Position = uProjectionMatrix * uViewMatrix * vec4(pos, 1.0);
}
/// ============= STUDENT CODE END =============
/// ===============================================================================================