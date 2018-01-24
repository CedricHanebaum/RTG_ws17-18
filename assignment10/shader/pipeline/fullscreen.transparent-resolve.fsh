#include "../common.glsl"
#include "../shading.glsl"

// defined in shading: uniform sampler2DRect uTexOpaqueDepth;
uniform sampler2DRect uTexShadedOpaque;
uniform sampler2DRect uTexTBufferAccumA;
uniform sampler2DRect uTexTBufferAccumB;
uniform sampler2DRect uTexTBufferDistortion;
// FIXME: uniform sampler2DRect uTexSSR;

uniform bool uDrawBackground;

in vec2 vPosition;

out vec3 fColor;

void main() 
{
    ivec2 coords = ivec2(gl_FragCoord.xy);

    vec3 opaqueColor = vec3(0);
    vec3 transparentColor = vec3(0);
    float alpha = 0.0;

    // read T-Buffer
    vec4 tAccumA = texelFetch(uTexTBufferAccumA, coords);
    float tAccumB = texelFetch(uTexTBufferAccumB, coords).x;

    // assemble weighted OIT
    alpha = 1 - tAccumA.a;
    vec4 accum = vec4(tAccumA.xyz, tAccumB);
    transparentColor = accum.rgb / clamp(accum.a, 1e-4, 5e4);

    // distortion
    vec3 tDistortion = texelFetch(uTexTBufferDistortion, coords).xyz;
    vec2 offset = tDistortion.xy;
    float blurriness = tDistortion.z;

    // (offset is in screen coordinates)
    opaqueColor = texture(uTexShadedOpaque, gl_FragCoord.xy + offset).rgb;
    float opaqueDepth = texture(uTexOpaqueDepth, gl_FragCoord.xy + offset).x;
    // TODO: blurriness

    // background
    if (uDrawBackground)
    {
        vec4 near = uInvProj * vec4(vPosition * 2 - 1, -1, 1);
        vec4 far = uInvProj * vec4(vPosition * 2 - 1, opaqueDepth * 2 - 1, 1);

        near /= near.w;
        far /= far.w;

        near = uInvView * near;
        far = uInvView * far;

        vec3 bgColor = texture(uTexCubeMap, (far - near).xyz).rgb;

        vec3 sunColor = vec3(0.9, 0.8, 0.7);

        float dotVL = max(0, dot(uLightDir, normalize(far - near).xyz));
        // Sun
        bgColor += 0.2 * smoothstep(0.996, 0.9999, dotVL) * sunColor;
        // Corona
        bgColor += mix(vec3(0), sunColor, pow(dotVL, 180));

        const float transition = 3.0;
        opaqueColor = mix(
            opaqueColor,
            bgColor,
            smoothstep(uRenderDistance - transition, uRenderDistance, distance(far.xyz, uCamPos))
        );
    }

    fColor = mix(opaqueColor, transparentColor, alpha);

    // FIXME: fColor += texelFetch(uTexSSR, ivec2(gl_FragCoord.xy * 0.5)).rgb;
}
