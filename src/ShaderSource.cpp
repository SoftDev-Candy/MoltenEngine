//
// Created by Candy on 11/21/2025.
//

#include "ShaderSource.hpp"

//This is like tiny C functions the language is called GLSL
const char* VertexShaderSource = R"(
#version 330 core
layout(location = 0) in vec3 aPos;
uniform mat4 MVP; //allows us to send model X view X projection from C++

void main() {
    gl_Position =MVP* vec4(aPos, 1.0);
}
)";

const char* FragmentShaderSource = R"(
#version 330 core
out vec4 FragColor;
void main() {
    FragColor = vec4(1.0, 0.5, 0.2, 1.0);
}
)";
