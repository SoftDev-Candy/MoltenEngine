//
// Created by Candy on 11/21/2025.
//

#include "Renderer.hpp"
#include <glad/glad.h>

void Renderer::Begin()
{
    glClearColor(.12f,0.13f,0.17f,1.0f); //The colour we want to give to the screen
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//Clearing the buffer to get the colour out.
}

void Renderer::RenderScene(const Scene &scene)
{

    for (auto* obj:scene.GetObjects())
    {
        obj->Render();
    }
}
