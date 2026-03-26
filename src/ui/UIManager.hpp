//
// Created by Candy on 12/18/2025.
//

#ifndef B_WENGINE_UIMANAGER_HPP
#define B_WENGINE_UIMANAGER_HPP
#include "../Camera.hpp"
#include "../Scene.hpp"
#include "imgui.h"
#include "ImGuizmo.h"
#include <functional>
#include <memory>
#include <string>
#include <vector>

class SplineShooterGame;

struct Message; // forward declare so UI doesn't drag in message includes everywhere

class UIManager
{

public:

    //THIS IS GOING TO GET MASSIVE BIG HEAD ! SNIPER'DREAM LARGE ENOUGH TO SEE FROM THE SUN !//
    void Draw(Scene& scene, Camera& camera, int& selectedIndex,

              //Read-only queries (UI can inspect assets, not mutate engine directly)
              std::function<Mesh*(const std::string&)> getMeshByKey,
              std::function<std::vector<std::string>()> listMeshKeys,
              std::function<std::string(const std::string&)> getMeshSourcePath,
              std::function<Texture*(const std::string&)> getTextureByKey,
              std::function<std::vector<std::string>()> listTextureKeys,
              std::function<std::string(const std::string&)> getTextureSourcePath,

              //THIS IS THE IMPORTANT BIT: UI pushes actions as Messages
              std::function<void(std::unique_ptr<Message>)> pushMessage);

    void DrawPlayHUD(const SplineShooterGame& game, std::function<void(std::unique_ptr<Message>)> pushMessage);

    void SetPerfStats(float fps, float ms)
    {
        fps_ = fps;
        ms_ = ms;
    }

    void LoadSaveSceneUI(std::function<void(std::unique_ptr<Message>)> pushMessage);


private:
    void DrawAssetWindow(
        Scene& scene,
        int selectedIndex,
        std::function<Mesh*(const std::string&)> getMeshByKey,
        std::function<std::vector<std::string>()> listMeshKeys,
        std::function<std::string(const std::string&)> getMeshSourcePath,
        std::function<Texture*(const std::string&)> getTextureByKey,
        std::function<std::vector<std::string>()> listTextureKeys,
        std::function<std::string(const std::string&)> getTextureSourcePath,
        std::function<void(std::unique_ptr<Message>)> pushMessage);
    void DrawViewportGizmo(Scene& scene, Camera& camera, int selectedIndex);

    bool showPerf_ = true;
    bool showMoveGizmo_ = true;
    ImGuizmo::OPERATION currentGizmoOperation_ = ImGuizmo::TRANSLATE;
    ImGuizmo::MODE currentGizmoMode_ = ImGuizmo::WORLD;
    float fps_ = 0.0f;
    float ms_  = 0.0f;

    float viewportMinX_ = 0.0f;
    float viewportMinY_ = 0.0f;
    float viewportMaxX_ = 0.0f;
    float viewportMaxY_ = 0.0f;
    bool viewportHovered_ = false;

    int activeGizmoAxis_ = -1;
    EntityID activeGizmoEntityId_ = 0;
    float gizmoStartMouseX_ = 0.0f;
    float gizmoStartMouseY_ = 0.0f;
    float gizmoPixelsPerUnit_ = 1.0f;
    float gizmoAxisScreenX_ = 1.0f;
    float gizmoAxisScreenY_ = 0.0f;
    glm::vec3 gizmoStartPosition_{0.0f, 0.0f, 0.0f};
    glm::vec3 gizmoAxisDirection_{1.0f, 0.0f, 0.0f};

    char assetSearch_[128] = "";
    char projectAssetSearch_[128] = "";

};

#endif //B_WENGINE_UIMANAGER_HPP
