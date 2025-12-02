//
// Created by Candy on 11/21/2025.
//

#include "Renderer.hpp"
#include "Scene.hpp"
#include <glad/glad.h>

void Renderer::Begin()
{
    glClearColor(.12f,0.13f,0.17f,1.0f); //The colour we want to give to the screen
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//Clearing the buffer to get the colour out.
}

void Renderer::RenderScene(Scene &scene , Camera& camera)
{
Camera& cam  = camera;
    for (auto& obj:scene.GetObjects())
    {

        if (obj.renderable)
        {
            glm::mat4 model = obj.transform.GetMatrix();
            glm::mat4 view = cam.GetView();
            glm::mat4 projection = cam.GetProjection(800.0f , 800.0f);

            glm::mat4 mvp = projection * view * model;
            obj.renderable->Render(mvp);
        }
    }
}
