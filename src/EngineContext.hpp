//
// Created by Candy on 11/21/2025.
//

#ifndef B_WENGINE_ENGINECONTEXT_HPP
#define B_WENGINE_ENGINECONTEXT_HPP
#include<iostream>
#include "Shader.hpp"
#include<glad/glad.h>
#include<GLFW/glfw3.h>

#include "Renderer.hpp"
#include "Scene.hpp"

class EngineContext
{

public:
void init();
    bool ShouldClose();
    void update();
    void Render();
    void Terminate();
    void AddObject(Renderable* object);

private:

GLFWwindow* window;
    Scene scene;
    Renderer renderer;

};


#endif //B_WENGINE_ENGINECONTEXT_HPP