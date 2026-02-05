//
// Created by Candy on 11/21/2025.
//

#include "Renderer.hpp"
#include "Texture.hpp"
#include <iostream>
#include "Scene.hpp"
#include <glad/glad.h>

#include "GLFW/glfw3.h"
void Renderer::SetShadowsEnabled(bool enabled)
{
    shadowsEnabled = enabled;
}

void Renderer::Begin()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    InitShadowResources();

    glClearColor(.12f,0.13f,0.17f,1.0f); //The colour we want to give to the screen
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);//Clearing the buffer to get the colour out.
}

void Renderer::RenderScene(Scene &scene , Camera& cam)
{
    glm::mat4 view = cam.GetView();
    int fbW = 0, fbH = 0;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
    glm::mat4 projection = cam.GetProjection((float)fbW, (float)fbH);
    // -----------------------------
    // SHADOW PASS (depth only)
    // -----------------------------
    auto& lights = scene.GetLights();
    glm::vec3 shadowLightPos = glm::vec3(2.0f, 3.0f, 2.0f);
    if (!lights.empty()) shadowLightPos = lights[0].position;

    glm::mat4 lightProj = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, 0.1f, 50.0f);
    glm::mat4 lightView = glm::lookAt(shadowLightPos, glm::vec3(0.0f), glm::vec3(0, 1, 0));
    glm::mat4 lightSpace = lightProj * lightView;

    if (shadowsEnabled && depthShader != nullptr)
    {
        glViewport(0, 0, shadowSize, shadowSize);
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
        glClear(GL_DEPTH_BUFFER_BIT);

        depthShader->bind();
        depthShader->setMat4("uLightSpaceMatrix", lightSpace);

        for (auto& o : scene.GetObjects())
        {
            if (o.mesh.mesh == nullptr) continue;

            glm::mat4 m = o.transform.GetMatrix();
            depthShader->setMat4("uModel", m);

            o.mesh.mesh->Bind();
            glDrawElements(GL_TRIANGLES, o.mesh.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // restore viewport for normal render
    glViewport(0, 0, fbW, fbH);


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
               shaderptr->setMat4("uLightSpaceMatrix", lightSpace);
               shaderptr->setInt("uShadowMap", 2);
               shaderptr->setInt("uShadowsEnabled", shadowsEnabled ? 1 : 0);

               // bind shadow depth texture to unit 2
               glActiveTexture(GL_TEXTURE2);
               glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

               auto& lights = scene.GetLights();

               int count = (int)lights.size();
               if (count > 8) count = 8;

               shaderptr->setInt("uLightCount", count);

               for (int i = 0; i < count; ++i)
               {
                   const auto& L = lights[i];

                   shaderptr->setVec3(std::string("uLightPos[") + std::to_string(i) + "]", L.position);
                   shaderptr->setVec3(std::string("uLightColor[") + std::to_string(i) + "]", L.color);
                   shaderptr->setFloat(std::string("uLightIntensity[") + std::to_string(i) + "]", L.intensity);
                   shaderptr->setFloat(std::string("uLightAmbient[") + std::to_string(i) + "]", L.ambientStrength);
                   // rotation is degrees -> convert
                   glm::vec3 rotRad = glm::radians(L.rotation);

                   glm::mat4 R(1.0f);
                   R = glm::rotate(R, rotRad.x, glm::vec3(1,0,0));
                   R = glm::rotate(R, rotRad.y, glm::vec3(0,1,0));
                   R = glm::rotate(R, rotRad.z, glm::vec3(0,0,1));

                   // OpenGL “forward” is typically -Z
                   glm::vec3 dir = glm::normalize(glm::vec3(R * glm::vec4(0,0,-1,0)));

                   shaderptr->setVec3("uLightDir[" + std::to_string(i) + "]", dir);


                   // angles are degrees in UI -> cos expects radians
                   float innerCos = cosf(glm::radians(L.innerAngle));
                   float outerCos = cosf(glm::radians(L.outerAngle));

                   shaderptr->setFloat(std::string("uLightInnerCos[") + std::to_string(i) + "]", innerCos);
                   shaderptr->setFloat(std::string("uLightOuterCos[") + std::to_string(i) + "]", outerCos);


               }

               // camera position (public in Camera class)
               shaderptr->setVec3("uViewPos", cam.position);

    shaderptr->setFloat("uShininess", obj.shininess);
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


            obj.mesh.mesh->Bind();
            glDrawElements(GL_TRIANGLES, obj.mesh.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

        }
    }
               // -----------------------------
               // LIGHT GIZMOS (so lights are visible)
               // -----------------------------
               Mesh* gizmoMesh = nullptr;
               // if you can access a cube mesh from scene objects: easiest fallback is draw using the first object's meshKey "Cube" isn't accessible here.
               // BEST minimal: store a pointer in Renderer (SetLightGizmoMesh) OR just reuse the currently bound obj mesh? Not good.
               // So: do the clean minimal: add a setter and set it from EngineContext (next step).
               if (lightGizmoMesh != nullptr)
               {
                   // Optional wireframe toggle
                   if (renderLightWireframe)
                       glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

                   shaderptr->bind(); // reuse same shader (it will still shade, good enough for demo)

                   auto& lights = scene.GetLights();
                   for (int i = 0; i < (int)lights.size() && i < 8; ++i)
                   {
                       const auto& L = lights[i];

                       glm::mat4 m = glm::mat4(1.0f);
                       m = glm::translate(m, L.position);
                       m = glm::rotate(m, glm::radians(L.rotation.x), glm::vec3(1,0,0));
                       m = glm::rotate(m, glm::radians(L.rotation.y), glm::vec3(0,1,0));
                       m = glm::rotate(m, glm::radians(L.rotation.z), glm::vec3(0,0,1));

                       m = glm::scale(m, glm::vec3(0.15f)); // small cube

                       glm::mat4 mvpGizmo = projection * view * m;

                       shaderptr->setMat4("uModel", m);
                       shaderptr->setMat4("MVP", mvpGizmo);

                       lightGizmoMesh->Bind();
                       glDrawElements(GL_TRIANGLES, lightGizmoMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
                   }

                   if (renderLightWireframe)
                       glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
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
void Renderer::InitShadowResources()
{
    if (shadowFBO != 0) return;

    glGenFramebuffers(1, &shadowFBO);

    glGenTextures(1, &shadowDepthTex);
    glBindTexture(GL_TEXTURE_2D, shadowDepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
                 shadowSize, shadowSize, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float border[4] = {1.f, 1.f, 1.f, 1.f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);

    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowDepthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
    {
        std::cout << "[Shadow] FBO not complete!\n";
    }

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
