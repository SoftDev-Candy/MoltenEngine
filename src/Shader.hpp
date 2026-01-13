//
// Created by Candy on 11/17/2025.
//

#ifndef B_WENGINE_SHADER_HPP
#define B_WENGINE_SHADER_HPP

#include <string>
#include<glad/glad.h>
#include "glm/fwd.hpp"
#include <glm/gtc/type_ptr.hpp>

class Shader
{
public:
//Build a shader program from the VertexShader as well as the FragmentShader SimpleStuff//
Shader(const char* VertexShader , const char* FragmentSrc);

//make this Shader the active one basically we are using this one here..will call glUseProgram(m_id)//
    void bind()const;
    void setMat4 (const std::string& name ,const glm::mat4& mat);
    void setInt(const char* texture , int id);



private:
GLuint m_id; //openGl shader id
GLuint CompileSingle(GLenum type , const char* src);

};

#endif //B_WENGINE_SHADER_HPP