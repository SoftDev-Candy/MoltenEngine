//
// Created by Candy on 11/21/2025.
//

#include "Renderer.hpp"
#include "Texture.hpp"
#include <iostream>
#include "Scene.hpp"
#include <glad/glad.h>

#include "GLFW/glfw3.h"

void Renderer::Begin()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);


    glClearColor(.12f,0.13f,0.17f,1.0f); //The colour we want to give to the screen
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//Clearing the buffer to get the colour out.
}

void Renderer::RenderScene(Scene &scene , Camera& cam)
{
    glm::mat4 view = cam.GetView();
    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
    glm::mat4 projection = cam.GetProjection((float)fbW, (float)fbH);

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
               shaderptr->setMat4("uModel", model);
                shaderptr->setMat4("MVP" ,mvp);
               shaderptr->setVec3("uLightPos", glm::vec3(2.0f, 3.0f, 2.0f));
               shaderptr->setVec3("uLightColor", glm::vec3(1.0f, 1.0f, 1.0f));

               // camerI mean it a position (you have it public in Camera)
               shaderptr->setVec3("uViewPos", cam.position);

               shaderptr->setFloat("uAmbientStrength", 0.15f);
               shaderptr->setFloat("uShininess", 32.0f);
               shaderptr->setFloat("uSpecStrength", 1.0);



            }
            // --- Material binding (Phase 2A core) ---
            Texture* albedoTex = nullptr;
            Texture* specTex   = nullptr;

            // Albedo priority: per-entity albedo -> old per-entity texture -> engine active -> default
            if (obj.albedo) albedoTex = obj.albedo;
            else if (obj.texture) albedoTex = obj.texture;
            else if (textureptr) albedoTex = textureptr;
            else albedoTex = defaultTexture;

            // Specular: per-entity specular -> fallback to albedo (better than null)
            if (obj.specular) specTex = obj.specular;
            else specTex = albedoTex;

            // Bind albedo to unit 0
            glActiveTexture(GL_TEXTURE0);
            if (albedoTex) albedoTex->Bind();

            // Bind specular to unit 1
            glActiveTexture(GL_TEXTURE1);
            if (specTex) specTex->Bind();

            // Tell shader which units to read
            shaderptr->setInt("uAlbedoMap", 0);
            shaderptr->setInt("uSpecularMap", 1);

            // (for now, you can keep fixed shininess or use obj.shininess once added)
            shaderptr->setFloat("uShininess", 32.0f);


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
