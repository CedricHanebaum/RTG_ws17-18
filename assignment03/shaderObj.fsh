uniform vec3 uColor;
uniform bool uSphere;

in vec2 vRelPos;
out vec3 fColor;

void main()
{
    if (uSphere && distance(vRelPos, vec2(0.5, 0.5)) > 0.5)
        discard;
        
    fColor = uColor;
}
