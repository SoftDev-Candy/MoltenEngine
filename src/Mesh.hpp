//
// Created by Candy on 12/11/2025.
//

#ifndef B_WENGINE_MESH_HPP
#define B_WENGINE_MESH_HPP
#include <glad/glad.h>

class Mesh
{
public :
    //Mesh Constructor
    Mesh(float* vertices, size_t vertSize, unsigned int* indices, size_t indexCount, int strideFloats = 5);
    void Bind();
    unsigned int GetIndexCount()const;

    //Mesh Destructor
    ~Mesh();
private:
    GLuint VAO{};
    GLuint VBO{};
    GLuint EBO{};
    unsigned int indexCount;
    int strideFloats = 5;


};


#endif //B_WENGINE_MESH_HPP