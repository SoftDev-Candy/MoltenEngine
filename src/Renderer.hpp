//
// Created by Candy on 11/21/2025.
//

#ifndef B_WENGINE_RENDER_HPP
#define B_WENGINE_RENDER_HPP
#include "Scene.hpp"
#include "Camera.hpp"
#include "Shader.hpp"
#include <glm/glm.hpp>


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
    void SetDepthShader(Shader* s);
    void SetShadowsEnabled(bool enabled);
    unsigned int GetShadowDepthTex() const { return shadowDepthTex; } // optional debug



private:
    Shader* shaderptr = nullptr;
    Texture* textureptr = nullptr;//TODO--This shall disappear when a material system is in place.//
    Shader* depthShader = nullptr;

    unsigned int shadowFBO = 0;
    unsigned int shadowDepthTex = 0;
    int shadowSize = 2048;
    bool shadowsEnabled = true;

    void InitShadowResources();


};


#endif //B_WENGINE_RENDER_HPP