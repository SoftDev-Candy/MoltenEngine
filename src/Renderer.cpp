//
// Created by Candy on 11/21/2025.
//

#include "Renderer.hpp"
#include "Texture.hpp"
#include <iostream>
#include "Scene.hpp"
#include <glad/glad.h>

void Renderer::Begin()
{
    glClearColor(.12f,0.13f,0.17f,1.0f); //The colour we want to give to the screen
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//Clearing the buffer to get the colour out.
}

void Renderer::RenderScene(Scene &scene , Camera& cam)
{
    glm::mat4 view = cam.GetView();
    glm::mat4 projection = cam.GetProjection(800.0f , 800.0f);

    for (auto& obj:scene.GetObjects())
    {
        if (obj.mesh.mesh ==nullptr)
        {
            continue;
        }
        else
        {
            //1.Compute Model
            glm::mat4 model = obj.transform.GetMatrix();

            //2.Compute MVP
            glm::mat4 mvp = projection * view * model;
            //bind shader with MVP
            if (shaderptr == nullptr)
            {
                std::cout<<"The activeShader is null ERROR in File Renderer.cpp (RenderScene)\n";
                continue;
            }
           else {
                shaderptr->bind();
                shaderptr->setMat4("MVP" ,mvp);
            }
            //texture Rendering//
            glActiveTexture(GL_TEXTURE0);
            //texture Rendering//
            glActiveTexture(GL_TEXTURE0);

            //Per-entity first, then fallback to engine texture, then fallback to default//
            Texture* useTex = nullptr;

            //1. If entity has its own texture use that//
            if (obj.texture != nullptr)
            {
                useTex = obj.texture;
            }
            //2. Else use whatever EngineContext set as "active" texture//
            else if (textureptr != nullptr)
            {
                useTex = textureptr;
            }
            //3. Else fallback default texture (optional)//
            else
            {
                useTex = defaultTexture;
            }

            //Bind if we actually have one//
            if (useTex != nullptr)
            {
                useTex->Bind();
            }

            //FIXME- This is materialState dunno what that means but need to move it out from here when i Do//
            shaderptr->setInt("uTexture", 0);

            obj.mesh.mesh->Bind();
            glDrawElements(GL_TRIANGLES, obj.mesh.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

        }
    }
}

void Renderer::SetActiveShader(Shader *s)
{
    shaderptr = s ;
}

void Renderer::SetActiveTexture(Texture *t)
{
    textureptr = t;
}
void Renderer::SetDefaultTexture(Texture* t)
{
    defaultTexture = t;
}
