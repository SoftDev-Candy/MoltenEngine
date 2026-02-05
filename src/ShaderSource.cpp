//
// Created by Candy on 11/21/2025.
//

#include "ShaderSource.hpp"

const char* DepthVertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;

uniform mat4 uLightSpaceMatrix;
uniform mat4 uModel;

void main()
{
    gl_Position = uLightSpaceMatrix * uModel * vec4(aPos, 1.0);
}
)";

const char* DepthFragmentShaderSource = R"(
#version 330 core
void main()
{
    // depth only
}
)";

// -----------------------------
// Main lighting shader (your phong + shadows)
// -----------------------------
const char* VertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

out vec2 vUV;
out vec3 vWorldPos;
out vec3 vNormal;
out vec4 vLightSpacePos;

uniform mat4 MVP;
uniform mat4 uModel;
uniform mat4 uLightSpaceMatrix;

void main()
{
    vUV = aUV;

    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;

    mat3 normalMat = transpose(inverse(mat3(uModel)));
    vNormal = normalize(normalMat * aNormal);

    vLightSpacePos = uLightSpaceMatrix * world;

    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

const char* FragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 vUV;
in vec3 vWorldPos;
in vec3 vNormal;
in vec4 vLightSpacePos;

uniform sampler2D uAlbedoMap;
uniform sampler2D uSpecularMap;

// shadow map
uniform sampler2D uShadowMap;
uniform bool  uShadowsEnabled;

uniform vec3 uViewPos;

#define MAX_LIGHTS 8
uniform int   uLightCount;
uniform vec3  uLightPos[MAX_LIGHTS];
uniform vec3  uLightColor[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform float uLightAmbient[MAX_LIGHTS];
uniform vec3  uLightDir[MAX_LIGHTS];
uniform float uLightInnerCos[MAX_LIGHTS];
uniform float uLightOuterCos[MAX_LIGHTS];

uniform float uShininess;
uniform float uSpecStrength;

// ---- shadow helpers ----
float ShadowFactor(vec4 lightSpacePos, vec3 N, vec3 L)
{
    vec3 proj = lightSpacePos.xyz / lightSpacePos.w;
    proj = proj * 0.5 + 0.5;

    if (proj.z > 1.0) return 0.0;
    if (proj.x < 0.0 || proj.x > 1.0 || proj.y < 0.0 || proj.y > 1.0) return 0.0;

    float bias = max(0.0025 * (1.0 - dot(N, L)), 0.0005);

    float shadow = 0.0;
    vec2 texelSize = 1.0 / vec2(textureSize(uShadowMap, 0));
    for (int x = -1; x <= 1; ++x)
    {
        for (int y = -1; y <= 1; ++y)
        {
            float closest = texture(uShadowMap, proj.xy + vec2(x, y) * texelSize).r;
            float current = proj.z - bias;
            shadow += (current > closest) ? 1.0 : 0.0;
        }
    }
    shadow /= 9.0;
    return shadow;
}

void main()
{
    vec3 albedo = texture(uAlbedoMap, vUV).rgb;
    float specMask = texture(uSpecularMap, vUV).r;

    vec3 N = normalize(vNormal);
    vec3 V = normalize(uViewPos - vWorldPos);

    vec3 colorOut = vec3(0.0);

    int count = clamp(uLightCount, 0, MAX_LIGHTS);

    for (int i = 0; i < count; ++i)
    {
        // --- distance attenuation (THIS is the missing piece) ---
        vec3  lightVec = uLightPos[i] - vWorldPos;
        float dist     = length(lightVec);
        vec3  L        = normalize(lightVec);

        // classic attenuation curve (tweak constants if you want)
        float atten = 1.0 / (1.0 + 0.09 * dist + 0.032 * dist * dist);

        vec3 lightCol = uLightColor[i] * uLightIntensity[i] * atten;

        // ambient (not affected by spotlight)
        vec3 ambient = uLightAmbient[i] * albedo * lightCol;

        // diffuse
        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * albedo * lightCol;

        // specular (Phong)
        vec3 R = reflect(-L, N);
        float spec = pow(max(dot(V, R), 0.0), uShininess);
        vec3 specular = (uSpecStrength * spec * specMask) * lightCol;

        // spotlight factor
        // Use -L because L points fragment -> light, but spotlight direction is light -> outward
        float cosTheta = dot(normalize(-L), normalize(uLightDir[i]));
        float spot = smoothstep(uLightOuterCos[i], uLightInnerCos[i], cosTheta);

        diffuse *= spot;
        specular *= spot;

        float shadow = 0.0;
        if (uShadowsEnabled && i == 0)
        {
            shadow = ShadowFactor(vLightSpacePos, N, L);
        }

        colorOut += ambient + (1.0 - shadow) * (diffuse + specular);
    }

    FragColor = vec4(colorOut, 1.0);
}
)";
