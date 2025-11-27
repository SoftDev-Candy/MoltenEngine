//
// Created by Candy on 11/22/2025.
//

#include "Cube.hpp"
#include "ShaderSource.hpp"
#include "GLFW/glfw3.h"

Cube::Cube() : shader(VertexShaderSource,FragmentShaderSource)
{
    float vertices[] =
    {
        -0.5f, -0.5f, -0.5f,
         0.5f, -0.5f, -0.5f,
         0.5f, 0.5f, -0.5f,
        -0.5f, 0.5f, -0.5f,

        -0.5f, -0.5f, 0.5f,
        0.5f, -0.5f, 0.5f,
        0.5f, 0.5f, 0.5f,
        -0.5f, 0.5f, 0.5f

    };

    unsigned int indices[] =
    {
        0, 1, 2, 2, 3, 0, // back face
        4, 5, 6, 6, 7, 4, // front face
        0, 4, 7, 7, 3, 0, // left face
        1, 5, 6, 6, 2, 1, // right face
        3, 2, 6, 6, 7, 3, // top face
        0, 1, 5, 5, 4, 0 // bottom face
    };

    //Create VAO
    glGenVertexArrays(1,&VAO);
    glBindVertexArray(VAO);

    //Then VBO
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER,VBO);
    glBufferData(GL_ARRAY_BUFFER,sizeof(vertices),vertices,GL_STATIC_DRAW);

    //Generate an EBO
    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,sizeof(indices),indices,GL_STATIC_DRAW);

    //lets openGl how to read to our data//
    glVertexAttribPointer(
        0, // attribute location 0 in shader
        3, // 3 components per vertex
        GL_FLOAT, // type
        GL_FALSE, // no normalization
        3 * sizeof(float), // stride (size of each vertex)
        (void *) 0 // offset (start of buffer)
    );
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

}

Cube::~Cube()
{
    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteBuffers(1, &EBO);

}

void Cube::Render()
{
    glm::mat4 model = glm::mat4(1.0f);
    model = glm::rotate (model,
          (float)glfwGetTime(),
        glm::vec3(0.5f,1.0f,0.0f));

    glm::mat4 view = glm::lookAt(
      glm::vec3(0.0f ,0.0f ,3.0f),
    glm::vec3(0.0f ,0.0f ,0.0f),
       glm::vec3(0.0f ,1.0f ,0.0f));

    glm::mat4 projection = glm::perspective(
         glm::radians(45.0f),
         800.f/800.0f,
         0.1f,
         100.f);
glm::mat4  mvp = projection * view * model;

    shader.bind();
    shader.setMat4("MVP" ,mvp); //TODO -ADD THIS FUNCTION IN SHADER//

    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES,36,GL_UNSIGNED_INT,0);
}
