//
// Created by Candy on 11/21/2025.
//

#ifndef B_WENGINE_RENDER_HPP
#define B_WENGINE_RENDER_HPP
#include "Scene.hpp"
#include "Camera.hpp"
#include "Shader.hpp"

class Texture;

class Renderer
{
public:

    void Begin();
    void RenderScene(Scene& scene , Camera& cam);
    void SetActiveShader(Shader *s) ;
    void SetActiveTexture(Texture* t); //FIXME: later this becomes per-entity / material system

    Texture* defaultTexture = nullptr;
    void SetDefaultTexture(Texture* t);


private:
    Shader* shaderptr = nullptr;
    Texture* textureptr = nullptr;//TODO--This shall disappear when a material system is in place.//


};


#endif //B_WENGINE_RENDER_HPP