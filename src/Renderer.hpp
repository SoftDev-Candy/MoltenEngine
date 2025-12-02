//
// Created by Candy on 11/21/2025.
//

#ifndef B_WENGINE_RENDER_HPP
#define B_WENGINE_RENDER_HPP
#include "Scene.hpp"
#include "Camera.hpp"
class Renderer
{
public:

    void Begin();
    void RenderScene(Scene& scene , Camera& camera);

};


#endif //B_WENGINE_RENDER_HPP