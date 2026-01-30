//
// Created by Candy on 12/7/2025.
//

#ifndef B_WENGINE_TEXTURE_HPP
#define B_WENGINE_TEXTURE_HPP

#include"glad/glad.h"


class Texture
{
public:
    Texture(const char* path);
    void Bind();
    GLuint id;

    void ApplySampling(bool useMipmaps);
    void EnsureMipmaps(); // builds mipmaps once
    bool hasMipmaps = false;


    ~Texture();

};


#endif //B_WENGINE_TEXTURE_HPP