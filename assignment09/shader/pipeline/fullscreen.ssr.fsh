#include "../common.glsl"
#include "../shading.glsl"

uniform sampler2DRect uTexShadedOpaque;


uniform mat4 uViewToPixel;

uniform vec3 uClipInfo;

in vec2 vPosition;

out vec3 fSSR;


void main()
{
    float uMarchingStepLimit = 40;

    vec4 gMatA = texture(uTexGBufferMatA, gl_FragCoord.xy);
    vec4 gMatB = texture(uTexGBufferMatB, gl_FragCoord.xy);

    float metallic = gMatA.w;
    float roughness = gMatB.x;

    float depth = texelFetch(uTexOpaqueDepth, ivec2(gl_FragCoord.xy)).r;
    vec4 screenPos = vec4(vPosition * 2 - 1, depth * 2 - 1, 1.0);
    vec4 viewPos = uInvProj * screenPos;
    viewPos /= viewPos.w;
    vec3 worldPos = vec3(uInvView * viewPos);

    ivec2 screenSize = textureSize(uTexShadedOpaque);

    vec3 worldNormal = normalize(gMatA.xyz * 2 - 1);
    vec3 eyeNormal = mat3(uView) * worldNormal;
    eyeNormal = normalize(eyeNormal);
    //fSSR = eyeNormal;
    //return;


    // Compute eye space position
    vec4 eyeSurfacePosition = viewPos;

    vec3 eyeViewingRay = normalize(eyeSurfacePosition.xyz);

    vec3 eyeReflectedViewingRay = reflect(eyeViewingRay, normalize(eyeNormal.xyz));
    eyeReflectedViewingRay      =  normalize(eyeReflectedViewingRay); // noop?

    // Project the surface and reflected surface position to screen space
    vec3 eyeReflectedSurfacePosition    = eyeSurfacePosition.xyz + eyeReflectedViewingRay;
    vec4 screenReflectedSurfacePosition = uProj * vec4(eyeReflectedSurfacePosition, 1.0);
    screenReflectedSurfacePosition.xyz /= screenReflectedSurfacePosition.w;
    vec4 screenSurfacePosition          = uProj * eyeSurfacePosition;
    screenSurfacePosition.xyz          /= screenSurfacePosition.w;

    // Map to [0, 1]
    screenReflectedSurfacePosition.xyz   = 0.5 * screenReflectedSurfacePosition.xyz + 0.5;
    screenSurfacePosition.xyz            = 0.5 * screenSurfacePosition.xyz + 0.5;

    // Compute screen space reflection vector
    vec3 screenReflectedViewingRay = screenReflectedSurfacePosition.xyz - screenSurfacePosition.xyz;

    vec2 uScreenSize = screenSize;
    // compute the scale factor, so that the screenReflectedViewingRay covers x pixels
    float scale = (2.0 / uScreenSize.x) / length(screenReflectedViewingRay.xy);

    vec3 screenReflectedViewingRayRef = screenReflectedViewingRay * scale;
    screenReflectedViewingRay *= scale;


    // now start the ray marching
    vec3 screenLastMarchingPosition     = screenSurfacePosition.xyz + screenReflectedViewingRay;
    vec3 screenCurrentMarchingPosition  = screenLastMarchingPosition.xyz + screenReflectedViewingRay;

    vec4 reflectionColor = vec4(0,0,0,0);

    float currentSampledDepth = 1.0;
    float lastSampledDepth    = -texture(uTexOpaqueDepth, screenSize * screenLastMarchingPosition.xy).x;

    int i = 0;
    while(screenCurrentMarchingPosition.x >= 0.0 && screenCurrentMarchingPosition.x <= 1.0 &&
        screenCurrentMarchingPosition.y >= 0.0 && screenCurrentMarchingPosition.y <= 1.0 &&
        i < uMarchingStepLimit && lastSampledDepth < 1.0)
    {
        currentSampledDepth = texture(uTexOpaqueDepth, screenCurrentMarchingPosition.xy * screenSize).x;
        if( lastSampledDepth > screenLastMarchingPosition.z &&
            screenCurrentMarchingPosition.z > currentSampledDepth &&
            abs(lastSampledDepth - currentSampledDepth) < abs(screenReflectedViewingRay.z * 1.55))
        {
            float denominator = ((currentSampledDepth - lastSampledDepth) - (screenCurrentMarchingPosition.z - screenLastMarchingPosition.z));
            float numA = (screenLastMarchingPosition.z - lastSampledDepth);
            float paramA = numA / denominator;

            if(denominator != 0.0 && 0 <= paramA && paramA <= 1)
                screenCurrentMarchingPosition.xy = screenLastMarchingPosition.xy + paramA * screenReflectedViewingRay.xy;
            else
                break;

            float fadeOut = 1.0;
            fadeOut *= 1.0 - max(abs(0.5 - screenCurrentMarchingPosition.x) - (0.5 - 0.2), 0.0) / 0.2;
            fadeOut *= 1.0 - max(abs(0.5 - screenCurrentMarchingPosition.y) - (0.5 - 0.2), 0.0) / 0.2;

            reflectionColor.rgb = texture(uTexShadedOpaque, screenSize.xy * screenCurrentMarchingPosition.xy).rgb;
            reflectionColor.a = 1.0;
            reflectionColor *= fadeOut;
            break;
        }
        else
        {
            screenLastMarchingPosition = screenCurrentMarchingPosition;
            lastSampledDepth = currentSampledDepth;
            screenCurrentMarchingPosition += screenReflectedViewingRay;
            screenReflectedViewingRayRef *= 1.05;

            float stepSize = 4.0;
            screenReflectedViewingRay = screenReflectedViewingRayRef * stepSize;
            ++i;
        }
    }


    vec4 near = uInvProj * vec4(vPosition * 2 - 1, -1, 1);
    vec4 far = uInvProj * vec4(vPosition * 2 - 1, depth * 2 - 1, 1);

    near /= near.w;
    far /= far.w;

    near = uInvView * near;
    far = uInvView * far;

    vec3 cubemap = texture(uTexCubeMap, (far - near).xyz).rgb;


    fSSR = (1-roughness) * metallic * mix(cubemap, reflectionColor.rgb, reflectionColor.a);
}
