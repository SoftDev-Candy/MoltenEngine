//
// Created by Candy on 2/16/2026.
//

#ifndef B_WENGINE_SCENESERIALIZER_HPP
#define B_WENGINE_SCENESERIALIZER_HPP

#include <string>

class Scene;
class Camera;

namespace SceneSerializer
{
    bool Save(const std::string& path, const Scene& scene, const Camera& cam,
              bool mipmapsEnabled, bool shadowsEnabled);

    bool Load(const std::string& path, Scene& scene, Camera& cam,
              bool& mipmapsEnabled, bool& shadowsEnabled);
}

#endif //B_WENGINE_SCENESERIALIZER_HPP