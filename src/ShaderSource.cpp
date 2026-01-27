//
// Created by Candy on 11/21/2025.
//

#include "ShaderSource.hpp"

//This is like tiny C functions the language is called GLSL
const char* VertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec2 aUV;
layout(location = 2) in vec3 aNormal;

out vec2 vUV;
out vec3 vWorldPos;
out vec3 vNormal;

uniform mat4 MVP;
uniform mat4 uModel;

void main() {
    vUV = aUV;

    vec4 world = uModel * vec4(aPos, 1.0);
    vWorldPos = world.xyz;

    mat3 normalMat = transpose(inverse(mat3(uModel)));
    vNormal = normalize(normalMat * aNormal);

    gl_Position = MVP * vec4(aPos, 1.0);
}
)";

const char* FragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;

in vec2 vUV;
in vec3 vWorldPos;
in vec3 vNormal;

uniform sampler2D uAlbedoMap;
uniform sampler2D uSpecularMap;

uniform vec3 uViewPos;

#define MAX_LIGHTS 8
uniform int   uLightCount;
uniform vec3  uLightPos[MAX_LIGHTS];
uniform vec3  uLightColor[MAX_LIGHTS];
uniform float uLightIntensity[MAX_LIGHTS];
uniform float uLightAmbient[MAX_LIGHTS];

uniform float uShininess;
uniform float uSpecStrength;

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
        vec3 L = normalize(uLightPos[i] - vWorldPos);

        vec3 lightCol = uLightColor[i] * uLightIntensity[i];

        // ambient
        vec3 ambient = uLightAmbient[i] * albedo * lightCol;

        // diffuse
        float diff = max(dot(N, L), 0.0);
        vec3 diffuse = diff * albedo * lightCol;

        // specular (Phong)
        vec3 R = reflect(-L, N);
        float spec = pow(max(dot(V, R), 0.0), uShininess);
        vec3 specular = (uSpecStrength * spec * specMask) * lightCol;

        colorOut += ambient + diffuse + specular;
    }

    FragColor = vec4(colorOut, 1.0);
}
)";
