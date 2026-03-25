//
// Created by Candy on 11/21/2025.
//

#include "Renderer.hpp"
#include "Texture.hpp"
#include <algorithm>
#include <cmath>
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
    int fbW = 0;
    int fbH = 0;
    glfwGetFramebufferSize(glfwGetCurrentContext(), &fbW, &fbH);
    glm::mat4 projection = cam.GetProjection((float)fbW, (float)fbH);

    // -----------------------------
    // SHADOW PASS (depth only)
    // -----------------------------
    auto& lights = scene.GetLights();
    glm::vec3 shadowLightPos = glm::vec3(2.0f, 3.0f, 2.0f);
    if (!lights.empty())
    {
        shadowLightPos = lights[0].position;
    }

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

        for (auto& sceneObject : scene.GetObjects())
        {
            if (sceneObject.mesh.mesh == nullptr)
            {
                continue;
            }

            glm::mat4 model = sceneObject.transform.GetMatrix();
            depthShader->setMat4("uModel", model);

            sceneObject.mesh.mesh->Bind();
            glDrawElements(GL_TRIANGLES, sceneObject.mesh.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }

        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glViewport(0, 0, fbW, fbH);

    if (shaderptr == nullptr)
    {
        std::cout << "The activeShader is null ERROR in File Renderer.cpp (RenderScene)\n";
        return;
    }

    auto ApplySharedSceneUniforms = [&]()
    {
        shaderptr->setMat4("uLightSpaceMatrix", lightSpace);
        shaderptr->setInt("uShadowMap", 2);
        shaderptr->setInt("uShadowsEnabled", shadowsEnabled ? 1 : 0);

        glActiveTexture(GL_TEXTURE2);
        glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

        int lightCount = (int)lights.size();
        if (lightCount > 8)
        {
            lightCount = 8;
        }

        shaderptr->setInt("uLightCount", lightCount);

        for (int lightIndex = 0; lightIndex < lightCount; ++lightIndex)
        {
            const auto& light = lights[lightIndex];

            shaderptr->setVec3(std::string("uLightPos[") + std::to_string(lightIndex) + "]", light.position);
            shaderptr->setVec3(std::string("uLightColor[") + std::to_string(lightIndex) + "]", light.color);
            shaderptr->setFloat(std::string("uLightIntensity[") + std::to_string(lightIndex) + "]", light.intensity);
            shaderptr->setFloat(std::string("uLightAmbient[") + std::to_string(lightIndex) + "]", light.ambientStrength);

            glm::vec3 rotationRadians = glm::radians(light.rotation);
            glm::mat4 rotationMatrix(1.0f);
            rotationMatrix = glm::rotate(rotationMatrix, rotationRadians.x, glm::vec3(1,0,0));
            rotationMatrix = glm::rotate(rotationMatrix, rotationRadians.y, glm::vec3(0,1,0));
            rotationMatrix = glm::rotate(rotationMatrix, rotationRadians.z, glm::vec3(0,0,1));

            glm::vec3 lightDirection = glm::normalize(glm::vec3(rotationMatrix * glm::vec4(0,0,-1,0)));
            shaderptr->setVec3("uLightDir[" + std::to_string(lightIndex) + "]", lightDirection);

            float innerCos = cosf(glm::radians(light.innerAngle));
            float outerCos = cosf(glm::radians(light.outerAngle));

            shaderptr->setFloat(std::string("uLightInnerCos[") + std::to_string(lightIndex) + "]", innerCos);
            shaderptr->setFloat(std::string("uLightOuterCos[") + std::to_string(lightIndex) + "]", outerCos);
        }

        shaderptr->setVec3("uViewPos", cam.position);
    };

    auto BindMaterialTextures = [&](Texture* albedoTexture, Texture* specularTexture)
    {
        glActiveTexture(GL_TEXTURE0);
        if (albedoTexture)
        {
            albedoTexture->Bind();
        }

        glActiveTexture(GL_TEXTURE1);
        if (specularTexture)
        {
            specularTexture->Bind();
        }

        shaderptr->setInt("uAlbedoMap", 0);
        shaderptr->setInt("uSpecularMap", 1);
    };

    for (auto& sceneObject : scene.GetObjects())
    {
        if (sceneObject.mesh.mesh == nullptr)
        {
            continue;
        }

        glm::mat4 model = sceneObject.transform.GetMatrix();
        glm::mat4 mvp = projection * view * model;

        shaderptr->bind();
        shaderptr->setMat4("uModel", model);
        shaderptr->setMat4("MVP", mvp);
        ApplySharedSceneUniforms();
        shaderptr->setInt("uUseTintColor", 0);
        shaderptr->setFloat("uShininess", sceneObject.shininess);
        shaderptr->setFloat("uSpecStrength", 1.0f);

        Texture* albedoTexture = nullptr;
        Texture* specularTexture = nullptr;

        if (sceneObject.albedo) albedoTexture = sceneObject.albedo;
        else if (sceneObject.texture) albedoTexture = sceneObject.texture;
        else if (textureptr) albedoTexture = textureptr;
        else albedoTexture = defaultTexture;

        if (sceneObject.specular) specularTexture = sceneObject.specular;
        else specularTexture = albedoTexture;

        BindMaterialTextures(albedoTexture, specularTexture);

        sceneObject.mesh.mesh->Bind();
        glDrawElements(GL_TRIANGLES, sceneObject.mesh.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
    }

    if (renderEmptyObjectHelpers && emptyObjectHelperMesh != nullptr)
    {
        //Empty object helper cubes so invisible scene ghosts stop pretending they are a valid workflow//
        glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glLineWidth(3.0f);

        shaderptr->bind();
        ApplySharedSceneUniforms();
        shaderptr->setInt("uUseTintColor", 1);
        shaderptr->setFloat("uShininess", 6.0f);
        shaderptr->setFloat("uSpecStrength", 0.0f);
        BindMaterialTextures(defaultTexture, defaultTexture);

        for (const auto& sceneObject : scene.GetObjects())
        {
            if (sceneObject.mesh.mesh != nullptr || sceneObject.meshKey != "None")
            {
                continue;
            }

            glm::vec3 helperTint =
                (sceneObject.name.rfind("SplinePoint_", 0) == 0)
                    ? glm::vec3(1.00f, 0.66f, 0.24f)
                    : glm::vec3(0.42f, 0.84f, 1.00f);

            glm::mat4 helperModel = sceneObject.transform.GetMatrix();
            glm::mat4 helperMvp = projection * view * helperModel;

            shaderptr->setMat4("uModel", helperModel);
            shaderptr->setMat4("MVP", helperMvp);
            shaderptr->setVec3("uTintColor", helperTint);

            emptyObjectHelperMesh->Bind();
            glDrawElements(GL_TRIANGLES, emptyObjectHelperMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }

        shaderptr->setInt("uUseTintColor", 0);
        glLineWidth(1.0f);
        glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    }

    if (lightGizmoMesh != nullptr)
    {
        if (renderLightWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        }

        shaderptr->bind();
        ApplySharedSceneUniforms();
        shaderptr->setInt("uUseTintColor", 1);
        shaderptr->setFloat("uShininess", 8.0f);
        shaderptr->setFloat("uSpecStrength", 0.0f);
        BindMaterialTextures(defaultTexture, defaultTexture);

        for (int lightIndex = 0; lightIndex < (int)lights.size() && lightIndex < 8; ++lightIndex)
        {
            const auto& light = lights[lightIndex];

            glm::mat4 gizmoModel = glm::mat4(1.0f);
            gizmoModel = glm::translate(gizmoModel, light.position);
            gizmoModel = glm::rotate(gizmoModel, glm::radians(light.rotation.x), glm::vec3(1,0,0));
            gizmoModel = glm::rotate(gizmoModel, glm::radians(light.rotation.y), glm::vec3(0,1,0));
            gizmoModel = glm::rotate(gizmoModel, glm::radians(light.rotation.z), glm::vec3(0,0,1));
            gizmoModel = glm::scale(gizmoModel, glm::vec3(0.32f)); //Bit chunkier so the light arrow is not microscopic sadness//

            glm::mat4 gizmoMvp = projection * view * gizmoModel;
            shaderptr->setMat4("uModel", gizmoModel);
            shaderptr->setMat4("MVP", gizmoMvp);
            shaderptr->setVec3("uTintColor", light.color);

            lightGizmoMesh->Bind();
            glDrawElements(GL_TRIANGLES, lightGizmoMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }

        shaderptr->setInt("uUseTintColor", 0);

        if (renderLightWireframe)
        {
            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }
    }

    if (selectionGizmoMesh != nullptr && renderSelectionGizmo)
    {
        glDisable(GL_DEPTH_TEST);

        shaderptr->bind();
        ApplySharedSceneUniforms();
        shaderptr->setInt("uUseTintColor", 1);
        shaderptr->setFloat("uShininess", 8.0f);
        shaderptr->setFloat("uSpecStrength", 0.0f);
        BindMaterialTextures(defaultTexture, defaultTexture);

        struct GizmoAxisDraw
        {
            glm::vec3 axisDirection;
            glm::vec3 tintColor;
        };

        GizmoAxisDraw gizmoAxes[] =
        {
            { glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.96f, 0.26f, 0.26f) },
            { glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.28f, 0.90f, 0.36f) },
            { glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.30f, 0.56f, 1.00f) }
        };

        glm::vec3 arrowRotations[] =
        {
            glm::vec3(0.0f, -90.0f, 0.0f),
            glm::vec3(90.0f, 0.0f, 0.0f),
            glm::vec3(0.0f, 180.0f, 0.0f)
        };

        float gizmoDistanceToCamera = glm::length(cam.position - selectionGizmoPosition);
        float handleLength = std::clamp(gizmoDistanceToCamera * 0.07f, 0.30f, 1.20f);
        float handleThickness = std::max(0.022f, handleLength * 0.05f);
        float tipSize = handleThickness * 1.55f;

        //These are scene gizmos on purpose now, not little ImGui noodles pretending to be 3D//
        for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
        {
            const auto& axis = gizmoAxes[axisIndex];
            glm::vec3 rodCenter = selectionGizmoPosition + axis.axisDirection * (handleLength * 0.5f);
            glm::vec3 rodScale(handleThickness, handleThickness, handleThickness);

            if (axis.axisDirection.x != 0.0f) rodScale.x = handleLength;
            if (axis.axisDirection.y != 0.0f) rodScale.y = handleLength;
            if (axis.axisDirection.z != 0.0f) rodScale.z = handleLength;

            glm::mat4 rodModel = glm::mat4(1.0f);
            rodModel = glm::translate(rodModel, rodCenter);
            rodModel = glm::scale(rodModel, rodScale);

            glm::mat4 rodMvp = projection * view * rodModel;
            shaderptr->setMat4("uModel", rodModel);
            shaderptr->setMat4("MVP", rodMvp);
            shaderptr->setVec3("uTintColor", axis.tintColor);

            selectionGizmoMesh->Bind();
            glDrawElements(GL_TRIANGLES, selectionGizmoMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

            Mesh* arrowMesh = selectionGizmoArrowMesh ? selectionGizmoArrowMesh : selectionGizmoMesh;
            glm::mat4 tipModel = glm::mat4(1.0f);
            tipModel = glm::translate(tipModel, selectionGizmoPosition + axis.axisDirection * handleLength);
            tipModel = glm::rotate(tipModel, glm::radians(arrowRotations[axisIndex].x), glm::vec3(1,0,0));
            tipModel = glm::rotate(tipModel, glm::radians(arrowRotations[axisIndex].y), glm::vec3(0,1,0));
            tipModel = glm::rotate(tipModel, glm::radians(arrowRotations[axisIndex].z), glm::vec3(0,0,1));
            tipModel = glm::scale(tipModel, glm::vec3(tipSize));

            glm::mat4 tipMvp = projection * view * tipModel;
            shaderptr->setMat4("uModel", tipModel);
            shaderptr->setMat4("MVP", tipMvp);
            shaderptr->setVec3("uTintColor", axis.tintColor);

            arrowMesh->Bind();
            glDrawElements(GL_TRIANGLES, arrowMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);
        }

        glm::mat4 centerModel = glm::mat4(1.0f);
        centerModel = glm::translate(centerModel, selectionGizmoPosition);
        centerModel = glm::scale(centerModel, glm::vec3(handleThickness * 1.55f));

        glm::mat4 centerMvp = projection * view * centerModel;
        shaderptr->setMat4("uModel", centerModel);
        shaderptr->setMat4("MVP", centerMvp);
        shaderptr->setVec3("uTintColor", glm::vec3(0.96f, 0.96f, 0.96f));

        selectionGizmoMesh->Bind();
        glDrawElements(GL_TRIANGLES, selectionGizmoMesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

        shaderptr->setInt("uUseTintColor", 0);
        glEnable(GL_DEPTH_TEST);
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
