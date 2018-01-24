/// Task 2.b
/// Weighted, Blended Order-Independent Transparency
///
/// Your job is to:
///     - read the T-Buffer
///     - calculate "transparentColor" and "alpha" based on the T-Buffer
///
/// Notes:
///     - remember that revealage and accum.a are swapped
///     - you may want to clamp the accumulated alpha between 1e-4 and 5e4
///
/// ============= STUDENT CODE BEGIN =============

// read T-Buffer
vec4 tAccumA = texelFetch(uTexTBufferAccumA, coords);
float tAccumB = texelFetch(uTexTBufferAccumB, coords).x;

// assemble weighted OIT
alpha = 1 - tAccumA.a;
vec4 accum = vec4(tAccumA.xyz, tAccumB);
transparentColor = accum.rgb / clamp(accum.a, 1e-4, 5e4);

/// ============= STUDENT CODE END =============



/// Task 2.c
/// opaque color with refraction
///
/// Your job is to:
///     - use the distortion part of the T-Buffer to implement fake refractions
///
/// Notes:
///     - the distortion is saved in pixel coordinates
///     - be sure to use bi-linear interpolation
///
/// ============= STUDENT CODE BEGIN =============

// distortion
vec3 tDistortion = texelFetch(uTexTBufferDistortion, coords).xyz;
vec2 offset = tDistortion.xy;
float blurriness = tDistortion.z;

// (offset is in screen coordinates)
opaqueColor = texture(uTexShadedOpaque, gl_FragCoord.xy + offset).rgb;
float opaqueDepth = texture(uTexOpaqueDepth, gl_FragCoord.xy + offset).x;

/// ============= STUDENT CODE END =============



/// Task 2.a
///
/// Your job is to:
///     - draw the background of the scene by changing opaqueColor
///     - the transition should be smooth (e.g. blend the last 3m)
///
/// Notes:
///     - the maximal render distance is stored in uRenderDistance (do not use far plane only!)
///     - the skybox is stored in uTexCubeMap
///     - uInvProj and uInvView contain inverse projection and view matrix
///     - vPosition contains screen coordinates from 0..1
///
/// ============= STUDENT CODE BEGIN =============

vec4 near = uInvProj * vec4(vPosition * 2 - 1, -1, 1);
vec4 far = uInvProj * vec4(vPosition * 2 - 1, opaqueDepth * 2 - 1, 1);

near /= near.w;
far /= far.w;

near = uInvView * near;
far = uInvView * far;

vec3 bgColor = texture(uTexCubeMap, (far - near).xyz).rgb;

vec3 sunColor = vec3(0.9, 0.9, 0.7);

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
       
/// ============= STUDENT CODE END =============
