//
// Created by Candy on 11/17/2025.
//
#include "Shader.hpp"
#include<iostream>

Shader::Shader(const char *VertexSrc, const char *FragmentSrc)
{
//We're Working here //
    // 1. compile the vertex and fragment shaders
GLuint vs = CompileSingle(GL_VERTEX_SHADER ,VertexSrc);
GLuint fs = CompileSingle(GL_FRAGMENT_SHADER ,FragmentSrc);

    //2.Create a program and link them together here
    m_id = glCreateProgram();        // create empty program object
    glAttachShader(m_id, vs);        // attach vertex shader
    glAttachShader(m_id, fs);        // attach fragment shader
    glLinkProgram(m_id);             // link into final GPU pipeline

    GLint success;
    GLchar infoLog[512];
    glGetProgramiv(m_id,GL_LINK_STATUS, &success);

    if (!success)
    {
        glGetProgramInfoLog(m_id,512,nullptr,infoLog);
        std::cerr<<"Shader program link error:\n"<<infoLog<<std::endl;
    }

    //Delete them puta they are as useful as a gas tank during a fire ...the big kind...🔥 boom boom i want you in..
glDeleteShader(vs);
glDeleteShader(fs);
}

void Shader::bind() const
{
glUseProgram(m_id);
}

void Shader::setMat4(const std::string &name, const glm::mat4& mat)
{
    GLint loc = glGetUniformLocation(m_id,name.c_str());
    glUniformMatrix4fv(loc,1,GL_FALSE ,glm::value_ptr(mat));
}

GLuint Shader::CompileSingle(GLenum type, const char *src)
{
    //1.Create an empty shader object GLuint is unsigned int in OpenGl terms//
    GLuint shader = glCreateShader(type);

    //2.Attach Source
    glShaderSource(shader, 1, &src, nullptr);

    //3.Compile
    glCompileShader(shader);

    //4.Check for error in case we made a mistake you know
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(shader,GL_COMPILE_STATUS, &success);

    if (!success)
    {
        glGetShaderInfoLog(shader,512,nullptr,infoLog);
        std::cerr<<"Shader Compiler Error:\n"<<infoLog<<std::endl;

    }

    //Returning here is the compiled shader's Id.
    return shader;
}
