//
// Created by Candy on 12/7/2025.
//

#include "Texture.hpp"
#include <iostream>
//Defining that macro//
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "glad/glad.h"
#include "glfw/glfw3.h"

//RAII is very important although could've named it literally anything else ...resource allocation is initialization...WDUM<???//
Texture::Texture(const char *path)
{
    int height;
    int width;
    int channels;

  unsigned char* storedata =  stbi_load(path , &width , &height, &channels,0);

    GLenum internalFormat = 0;
    GLenum dataFormat = 0;
if (channels == 3)
{
    internalFormat =GL_RGB;
        dataFormat = GL_RGB;
}
else if (channels == 4)
{
    internalFormat =GL_RGBA;
    dataFormat = GL_RGBA;
}
    else
    {
        std::cerr<<"Error in texture handling for colour formats (Texture.cpp)"<<std::endl;
    }

        if(storedata == NULL)
        {
            std::cerr<<"The data for texture was NULL (Texture.cpp)"<<path<<std::endl;
        }

    else
    {
        std::cout<<"Loaded Texture "<<width <<" x "<<height<<"Channels "<<channels<<std::endl;
    }


    //First we generate the texture
    glGenTextures(1,&id);
    glBindTexture(GL_TEXTURE_2D, id);

glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_S,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_WRAP_T,GL_REPEAT);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR_MIPMAP_LINEAR);
glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);


    glTexImage2D(
     GL_TEXTURE_2D,     // always GL_TEXTURE_2D for us
     0,                 // mipmap level (always 0 for now) don't have any basically
     internalFormat,    // how GPU should store the texture (RGB? RGBA?) imp shit
     width, height,     // image size
     0,                 // Border = always 0 for now
     dataFormat,        // how OUR image is laid out (RGB or RGBA) thats how we will see it basically
     GL_UNSIGNED_BYTE,  // GL_UNSIGNED_BYTE (stbi loads 8-bit)
     storedata          // the pixel pointer (storedata)
 );
    glGenerateMipmap(GL_TEXTURE_2D);
    stbi_set_flip_vertically_on_load(true);


    stbi_image_free(storedata);

}

void Texture::Bind()
{
    glBindTexture(GL_TEXTURE_2D,id);

}

Texture::~Texture()
{

}
