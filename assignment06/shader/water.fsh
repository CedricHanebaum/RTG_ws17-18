#include "shading.glsl"
#include "noise2D.glsl"
#include "fluid.glsl"

uniform bool uIsShadowPass;

uniform sampler2DRect uFramebufferOpaque;
uniform sampler2DRect uFramebufferDepth;

uniform sampler2D uTexWaterNormal;

uniform float uRuntime;

uniform vec3 uCamPos;
uniform mat4 uInvProj;
uniform mat4 uProj;
uniform mat4 uView;

uniform int uRenderMode;

in vec2 vFluidCoords;
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vTangent;
in vec3 vViewPos;
in vec4 vScreenPos;

out vec3 fColor;

/// (Bonus Task)
/// Fix the refraction artifacts that are visible when camera is a bit further away
/// from the pool and looking flat on the water surface
///
/// Your job is to:
///     - fix what we did not manage in time
///
/// Notes:
///     - you may do whatever you want (also write outside this strip or shader)
///       but you should indicate (in the strip) where exactly you changed code
///     - you should comment your code
///     - refractions should still look plausible
///
/// ============= STUDENT CODE BEGIN =============
vec3 waterShading()
{
    vec3 L = normalize(uLightDir);
    vec3 N = normalize(vNormal);
    vec3 V = normalize(uCamPos - vWorldPos);
    vec3 T = normalize(vTangent);
    vec3 B = normalize(cross(T, N));
    mat3 tbn = mat3(T, B, N);
    vec3 ON = N;
    vec3 viewN = mat3(uView) * N;
    
    // material
    vec3 normalMap = texture(uTexWaterNormal, 0.01*vFluidCoords + uRuntime*0.01).rgb;
    vec3 specular = vec3(0.3);
    float roughness = 0.15;
    float reflectivity = 1.0;
    vec3 waterColor = vec3(0, 1, 1);
    float depthFalloff = 8.0;
    float cubeMapLOD = 3.0;
    float normalSmoothing = 3.0;

    // normal mapping
    normalMap.z *= normalSmoothing; // make normal map more "moderate"
    normalMap.xy = normalMap.xy * 2 - 1;
    N = normalize(tbn * normalMap);

    // reflection
    vec3 R = reflect(-V, N);
    vec3 cubeMapRefl = textureLod(uCubeMap, R, cubeMapLOD).rgb;

    // F (Fresnel term)
    float dotNV = max(dot(ON, V), 0.0);
    float F_a = 1.0;
    float F_b = pow(1.0 - dotNV, 5);
    vec3 F = mix(vec3(F_b), vec3(F_a), specular);

    // read opaque Framebuffer
    vec2 screenXY = vScreenPos.xy / vScreenPos.w;
    float opaqueDepth = texelFetch(uFramebufferDepth, ivec2(gl_FragCoord.xy)).x;
    vec4 opaquePos4 = uInvProj * vec4(screenXY, opaqueDepth * 2 - 1, 1.0);
    float opaqueDis = opaquePos4.z / opaquePos4.w;
    float waterDepth = vViewPos.z - opaqueDis;

    // custom z test
    if (gl_FragCoord.z > opaqueDepth)
        discard;

    // water lighting
    //  - reflection:
    //      - GGX
    //      - Skybox
    //  - refraction:
    //      - opaque sample (depth-modulated, tinted)
    vec3 color = vec3(0);

    // reflection
    vec3 refl = shadingSpecularGGXNoF(N, V, L, roughness, specular) * shadowing(vWorldPos, L) * uLightColor;
    refl += cubeMapRefl * reflectivity;

    // refraction
    float maxDepth = waterDepth;
    float minDepth = 0.0;
    vec2 tsize = vec2(textureSize(uFramebufferOpaque));
    vec3 RF = refract(-V, N, 1 / 1.33);
    vec3 groundPosR = vWorldPos + RF * waterDepth;
    vec4 groundPosS = uProj * uView * vec4(groundPosR, 1.0);
    vec2 refrXY = groundPosS.xy / groundPosS.w;
    vec2 refrXYS = (refrXY * 0.5 + 0.5) * tsize;
    vec3 opaqueColor = texture(uFramebufferOpaque, refrXYS).rgb;

    // depth modulation
    vec3 refr = mix(waterColor, opaqueColor, pow(0.5, waterDepth / depthFalloff));

    // return mix of refl and refr
    return mix(refr, refl, F);
}
/// ============= STUDENT CODE END =============

void main()
{
    // early-out for shadow rendering
    if (uIsShadowPass)
    {
        fColor = vec3(1, 0, 1);
        return;
    }


    if (uRenderMode == 0)
    {
        // "Realistic" shading
        fColor = waterShading();
    }
    else if (uRenderMode == 1)
    {
        // Visualize normals
        vec3 normal = normalize(vNormal);
        fColor = (0.5 + 0.5 * dot(uLightDir, normal)) * normal;
    }
    else if (uRenderMode == 2)
    {
        // Checker
        vec3 col0 = vec3(0.00, 0.33, 0.66);
        vec3 col1 = vec3(0.66, 0.00, 0.33);
        ivec2 intcoords = ivec2(vFluidCoords);
        vec3 checkerColor = float((intcoords.x+intcoords.y)%2)*col0 + float((intcoords.x+intcoords.y+1)%2)*col1;
        fColor = dot(uLightDir, normalize(vNormal)) * checkerColor;
    }
    else if (uRenderMode == 3)
    {
        // Flux
        float fluxX = fluxXF(vFluidCoords.x, vFluidCoords.y);
        float fluxY = fluxYF(vFluidCoords.x, vFluidCoords.y);

        // Clamp absolute value to 1
        fluxX = min(abs(fluxX), 1.0);
        fluxY = min(abs(fluxY), 1.0);

        vec3 col0 = vec3(1,0,0);
        vec3 col1 = vec3(0,1,0);
        vec3 fluxColor = fluxX * col0 + fluxY * col1;
        fColor = fluxColor;
    }
    else if (uRenderMode == 4)
    {
        // Divergence
        float div = divF(vFluidCoords.x, vFluidCoords.y);
        float posDiv = max(div, 0.0);
        float negDiv = max(-div, 0.0);
        vec3 col0 = vec3(0.00, 0.33, 0.66);
        vec3 col1 = vec3(0.66, 0.00, 0.33);
        fColor = posDiv * col0 + negDiv * col1;
    }
    else
    {
        // Unsupported mode
        float drawCircle = float(distance(vFluidCoords, 0.5 * vec2(uFluidSizeX, uFluidSizeY)) < 25);
        float width = 5;
        float length = width * 5;
        float x = vFluidCoords.x;
        float y = vFluidCoords.y;
        bool inCross1 = (abs(x+y- 0.5*(uFluidSizeX+uFluidSizeY)) < width && abs((uFluidSizeX-x)+y- 0.5*(uFluidSizeX+uFluidSizeY)) < length);
        inCross1 = inCross1 || (abs(x+y- 0.5*(uFluidSizeX+uFluidSizeY)) < width && abs((uFluidSizeX-x)+y- 0.5*(uFluidSizeX+uFluidSizeY)) < length);
        bool inCross2 = (abs(x+y- 0.5*(uFluidSizeX+uFluidSizeY)) < length && abs((uFluidSizeX-x)+y- 0.5*(uFluidSizeX+uFluidSizeY)) < width);
        inCross2 = inCross1 || (abs(x+y- 0.5*(uFluidSizeX+uFluidSizeY)) < length && abs((uFluidSizeX-x)+y- 0.5*(uFluidSizeX+uFluidSizeY)) < width);

        float drawCross = float(inCross1 || inCross2);
        vec3 col0 = vec3(max(drawCross, drawCircle), drawCross, drawCross);
        fColor = dot(uLightDir, normalize(vNormal)) * col0;
    }
}
