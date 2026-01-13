//
// Created by Candy on 12/18/2025.
//

#ifndef B_WENGINE_UIMANAGER_HPP
#define B_WENGINE_UIMANAGER_HPP
#include "../Camera.hpp"
#include "../Scene.hpp"
#include <functional>
#include <string>
#include <vector>


class UIManager
{

public:

    //THIS IS GOING TO GET MASSIVE BIG HEAD ! SNIPER'DREAM LARGE ENOUGH TO SEE FROM THE SUN !//
    void Draw(Scene& scene, Camera& camera, int& selectedIndex,
              std::function<Entity(const std::string&)> createCube,
              std::function<Entity(const std::string&)> createImported,
              std::function<Mesh*(const std::string&)> getMeshByKey,
              std::function<std::vector<std::string>()> listMeshKeys,
              std::function<bool(const std::string&, const std::string&)> importObjMesh,
              std::function<Texture*(const std::string&)> getTextureByKey,
              std::function<std::vector<std::string>()> listTextureKeys,
              std::function<bool(const std::string&, const std::string&)> importTexture);

private:

};

#endif //B_WENGINE_UIMANAGER_HPP