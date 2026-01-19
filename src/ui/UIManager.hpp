//
// Created by Candy on 12/18/2025.
//

#ifndef B_WENGINE_UIMANAGER_HPP
#define B_WENGINE_UIMANAGER_HPP
#include "../Camera.hpp"
#include "../Scene.hpp"
#include <functional>
#include <memory>
#include <string>
#include <vector>


struct Message; // forward declare so UI doesn't drag in message includes everywhere

class UIManager
{

public:

    //THIS IS GOING TO GET MASSIVE BIG HEAD ! SNIPER'DREAM LARGE ENOUGH TO SEE FROM THE SUN !//
    void Draw(Scene& scene, Camera& camera, int& selectedIndex,

              //Read-only queries (UI can inspect assets, not mutate engine directly)
              std::function<Mesh*(const std::string&)> getMeshByKey,
              std::function<std::vector<std::string>()> listMeshKeys,
              std::function<Texture*(const std::string&)> getTextureByKey,
              std::function<std::vector<std::string>()> listTextureKeys,

              //THIS IS THE IMPORTANT BIT: UI pushes actions as Messages
              std::function<void(std::unique_ptr<Message>)> pushMessage);

private:

};

#endif //B_WENGINE_UIMANAGER_HPP