//
// Created by Candy on 11/21/2025.
//

#ifndef B_WENGINE_ENGINECONTEXT_HPP
#define B_WENGINE_ENGINECONTEXT_HPP
#include<iostream>
#include<glad/glad.h>
#include<GLFW/glfw3.h>
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "MeshManager.hpp"
#include "Shader.hpp"
#include "ShaderManager.hpp"
#include "ui/UIManager.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "message/MessageQueue.hpp"
#include "message/Message.hpp"
#include <memory>

class EngineContext
{

public:
//Creating Camera object
Camera camera;
    Entity CreateCube(const std::string& cubename);
    Entity CreateEntityWithMesh(const std::string& name, const std::string& meshKey);

    void init();
    bool ShouldClose();
    void update();
    void Render();
    void Terminate();
    void AddObject();

    std::vector<SceneObject>& Getobject() ;

    //Texture helper Functions
    bool ImportObjAsMesh(const std::string& key, const std::string& path);
    bool ImportTexture(const std::string& key, const std::string& path);

    //Mesh and Texture objs
    Mesh* cubeMesh;
    Texture* texture;

    //Respective Manager Objects
    TextureManager textureManager;
    ShaderManager shadermanager;
    MeshManager meshmanager;
    UIManager ui;
    MessageQueue messagequeue;


    //Delete Function
    bool DeleteSelectedObject();
    bool DeleteEntity(Entity e);


    void PushMessage(std::unique_ptr<Message> msg);
    void ProcessMessages();
    void SetEntityMesh(Entity e, const std::string& meshKey);
    void SetEntityTexture(Entity e, const std::string& textureKey);


    Scene& GetScene() { return scene; }
    const Scene& GetScene() const { return scene; }


private:

    GLFWwindow* window;
    Scene scene;
    Renderer renderer;
    int selectedIndex = -1;

};

#endif //B_WENGINE_ENGINECONTEXT_HPP