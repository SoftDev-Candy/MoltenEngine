//
// Created by Candy on 12/11/2025.
//

#include "Mesh.hpp"

#include <iostream>

Mesh::Mesh(float *vertices, size_t vertSize, unsigned int *indices, size_t indexCount, int strideFloats)
{
    std::cout << "[Mesh] strideFloats=" << strideFloats
          << " vertFloats=" << vertSize
          << " indices=" << indexCount << "\n";

    this->indexCount = indexCount;
    //Create VAO Here //
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    //Generate VBO
    glGenBuffers(1,&VBO);
    glBindBuffer(GL_ARRAY_BUFFER , VBO);
    glBufferData(GL_ARRAY_BUFFER,vertSize*sizeof(float),vertices,GL_STATIC_DRAW);

    //Generate the EBO
    glGenBuffers(1,&EBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,indexCount*sizeof(unsigned int),indices,GL_STATIC_DRAW);

    //lets openGl know how to read to our data//
    // position
    // position (always)
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        strideFloats * sizeof(float),
        (void*)0
    );
    glEnableVertexAttribArray(0);

    // uv (always in your engine)
    glVertexAttribPointer(
        1, 2, GL_FLOAT, GL_FALSE,
        strideFloats * sizeof(float),
        (void*)(3 * sizeof(float))
    );
    glEnableVertexAttribArray(1);

    // normal (only if present)
    if (strideFloats >= 8)
    {
        glVertexAttribPointer(
            2, 3, GL_FLOAT, GL_FALSE,
            strideFloats * sizeof(float),
            (void*)(5 * sizeof(float))
        );
        glEnableVertexAttribArray(2);
    }


    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER,0);

}

void Mesh::Bind()
{
glBindVertexArray(VAO);
}

unsigned int Mesh::GetIndexCount() const
{
    return indexCount;
}

Mesh::~Mesh()
{
    glDeleteVertexArrays(1,&VAO);
    glDeleteBuffers(1,&VBO);
    glDeleteBuffers(1, &EBO);

}
