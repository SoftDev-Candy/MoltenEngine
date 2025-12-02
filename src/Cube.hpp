//
// Created by Candy on 11/22/2025.
//

#ifndef B_WENGINE_CUBE_HPP
#define B_WENGINE_CUBE_HPP
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include"Renderable.hpp"
#include "Shader.hpp"

class Cube :public Renderable
{
public:

    Cube();//Constructor big man //
    ~Cube();//No so constructor//

    void Render(const glm::mat4& mvp)override;

private:
    unsigned int VAO;
    unsigned int VBO;
    unsigned int EBO;

Shader shader;

};


#endif //B_WENGINE_CUBE_HPP