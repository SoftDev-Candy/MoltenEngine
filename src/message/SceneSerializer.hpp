//
// Created by Candy on 2/16/2026.
//

#ifndef B_WENGINE_SCENESERIALIZER_HPP
#define B_WENGINE_SCENESERIALIZER_HPP

#include <string>
#include <vector>
#include <unordered_set>
#include "../MeshManager.hpp"
#include "../TextureManager.hpp"

class TextureManager;
class MeshManager;
class Scene;
class Camera;

namespace SceneSerializer
{
    bool Save(const std::string& path, Scene& scene, const Camera& cam,
          bool mipmapsEnabled, bool shadowsEnabled,
          const MeshManager& meshMgr,
          const TextureManager& texMgr);

    bool Load(const std::string& path, Scene& scene, Camera& cam,
              bool& mipmapsEnabled, bool& shadowsEnabled,
              std::vector<std::pair<std::string,std::string>>& outMeshes,
              std::vector<std::pair<std::string,std::string>>& outTextures);
}

#endif //B_WENGINE_SCENESERIALIZER_HPP