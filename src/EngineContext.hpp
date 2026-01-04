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

class EngineContext
{

public:
//Creating Camera object
Camera camera;
    Entity CreateCube(const std::string& cubename);
    void init();
    bool ShouldClose();
    void update();
    void Render();
    void Terminate();
    void AddObject();
    Mesh* cubeMesh;
    ShaderManager shadermanager;
    UIManager ui;
    MeshManager meshmanager;

std::vector<SceneObject>& Getobject() ;

private:

    GLFWwindow* window;
    Scene scene;
    Renderer renderer;
    int selectedIndex = -1;

};

#endif //B_WENGINE_ENGINECONTEXT_HPP