//
// Created by Candy on 12/18/2025.
//  UIManager: draws the editor UI panels + lets you spawn stuff and edit it
//

#include "UIManager.hpp"

#include "EditorWidgets.hpp"
#include "../../game/GameUI.hpp"
#include "imgui.h"
#include "imgui_impl_opengl3.h"
#include <algorithm>
#include <array>
#include <cmath>
#include <cstring> // strcpy
#include <filesystem>
#include <string>  // std::to_string
#include <vector>
#include "../message/CreateEntityMessage.hpp"
#include "../message/DeleteEntityMessage.hpp"
#include "../message/SetEntityMeshMessage.hpp"
#include "../message/SetEntityTextureMessage.hpp"
#include "../message/ImportMeshMessage.hpp"
#include "../message/ImportTextureMessage.hpp"
#include "../message/AddLightMessage.hpp"
#include "../message/DeleteLightMessage.hpp"
#include "../message/UpdateLightMessage.hpp"
#include "../message/SetEntityAlbedoMessage.hpp"
#include "../message/SetEntitySpecularMessage.hpp"
#include "../message/SetEntityShininessMessage.hpp"
#include "../message/SetMipmapsEnabledMessage.hpp"
#include "../message/SetShadowsEnabledMessage.hpp"
#include "../message/SaveSceneMessage.hpp"
#include "../message/LoadSceneMessage.hpp"
#include "../message/StartGameMessage.hpp"
#include "../message/StopGameMessage.hpp"

namespace fs = std::filesystem;

static std::string GetFileStem(const char* path)
{
    //FIXME: this is a tiny helper so you dont have to type keys like a caveman//
    std::string s = path ? path : "";
    if (s.empty()) return "Asset";

    size_t slash = s.find_last_of("/\\");
    if (slash != std::string::npos) s = s.substr(slash + 1);

    size_t dot = s.find_last_of('.');
    if (dot != std::string::npos) s = s.substr(0, dot);

    if (s.empty()) s = "Asset";
    return s;
}

static void CopyStringToBuffer(const std::string& text, char* buffer, size_t bufferSize)
{
    if (bufferSize == 0)
    {
        return;
    }

    std::strncpy(buffer, text.c_str(), bufferSize);
    buffer[bufferSize - 1] = '\0';
}

static std::string ToLowerCopy(const std::string& text)
{
    std::string lowered = text;
    std::transform(
        lowered.begin(),
        lowered.end(),
        lowered.begin(),
        [](unsigned char character)
        {
            return (char)std::tolower(character);
        });
    return lowered;
}

static bool ContainsCaseInsensitive(const std::string& haystack, const std::string& needle)
{
    if (needle.empty())
    {
        return true;
    }

    return ToLowerCopy(haystack).find(ToLowerCopy(needle)) != std::string::npos;
}

static bool IsMeshAssetPath(const fs::path& path)
{
    return ToLowerCopy(path.extension().string()) == ".obj";
}

static bool IsTextureAssetPath(const fs::path& path)
{
    std::string extension = ToLowerCopy(path.extension().string());
    return extension == ".png" || extension == ".jpg" || extension == ".jpeg" ||
           extension == ".bmp" || extension == ".tga";
}

static int CountMeshUsage(Scene& scene, const std::string& meshKey)
{
    int usageCount = 0;
    for (const auto& sceneObject : scene.GetObjects())
    {
        if (sceneObject.meshKey == meshKey)
        {
            usageCount++;
        }
    }
    return usageCount;
}

static int CountTextureUsage(Scene& scene, const std::string& textureKey)
{
    int usageCount = 0;
    for (const auto& sceneObject : scene.GetObjects())
    {
        if (sceneObject.textureKey == textureKey ||
            sceneObject.albedoKey == textureKey ||
            sceneObject.specularKey == textureKey)
        {
            usageCount++;
        }
    }
    return usageCount;
}

static bool PointInRect(const ImVec2& point, const ImVec2& minCorner, const ImVec2& maxCorner)
{
    return point.x >= minCorner.x && point.x <= maxCorner.x &&
           point.y >= minCorner.y && point.y <= maxCorner.y;
}

static float DistanceToLineSegment(const ImVec2& point, const ImVec2& segmentStart, const ImVec2& segmentEnd)
{
    ImVec2 segment = ImVec2(segmentEnd.x - segmentStart.x, segmentEnd.y - segmentStart.y);
    ImVec2 toPoint = ImVec2(point.x - segmentStart.x, point.y - segmentStart.y);

    float segmentLengthSquared = segment.x * segment.x + segment.y * segment.y;
    if (segmentLengthSquared <= 0.0001f)
    {
        float dx = point.x - segmentStart.x;
        float dy = point.y - segmentStart.y;
        return std::sqrt(dx * dx + dy * dy);
    }

    float t = (toPoint.x * segment.x + toPoint.y * segment.y) / segmentLengthSquared;
    t = std::clamp(t, 0.0f, 1.0f);

    ImVec2 closestPoint = ImVec2(segmentStart.x + segment.x * t, segmentStart.y + segment.y * t);
    float dx = point.x - closestPoint.x;
    float dy = point.y - closestPoint.y;
    return std::sqrt(dx * dx + dy * dy);
}

static bool ProjectWorldToScreen(
    const glm::vec3& worldPosition,
    const Camera& camera,
    const ImVec2& screenSize,
    ImVec2& screenPosition)
{
    if (screenSize.x <= 1.0f || screenSize.y <= 1.0f)
    {
        return false;
    }

    glm::mat4 projection = camera.GetProjection(screenSize.x, screenSize.y);
    glm::mat4 view = camera.GetView();
    glm::vec4 clipPosition = projection * view * glm::vec4(worldPosition, 1.0f);

    if (clipPosition.w <= 0.0001f)
    {
        return false;
    }

    glm::vec3 ndcPosition = glm::vec3(clipPosition) / clipPosition.w;
    if (ndcPosition.z < -1.2f || ndcPosition.z > 1.2f)
    {
        return false;
    }

    screenPosition.x = (ndcPosition.x * 0.5f + 0.5f) * screenSize.x;
    screenPosition.y = (1.0f - (ndcPosition.y * 0.5f + 0.5f)) * screenSize.y;
    return true;
}

static glm::vec3 GetSceneGizmoAnchor(const SceneObject& sceneObject)
{
    glm::vec3 halfScale = glm::abs(sceneObject.transform.scale) * 0.5f;
    glm::vec3 rotationRadians = glm::radians(sceneObject.transform.rotation);
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    glm::mat4 rotationMatrix(1.0f);
    rotationMatrix = glm::rotate(rotationMatrix, rotationRadians.x, glm::vec3(1,0,0));
    rotationMatrix = glm::rotate(rotationMatrix, rotationRadians.y, glm::vec3(0,1,0));
    rotationMatrix = glm::rotate(rotationMatrix, rotationRadians.z, glm::vec3(0,0,1));

    glm::vec3 rotatedXAxis = glm::vec3(rotationMatrix * glm::vec4(1,0,0,0));
    glm::vec3 rotatedYAxis = glm::vec3(rotationMatrix * glm::vec4(0,1,0,0));
    glm::vec3 rotatedZAxis = glm::vec3(rotationMatrix * glm::vec4(0,0,1,0));

    //We fake a cheap support-point here so the gizmo chills above the object instead of in its ribcage//
    float topOffset =
        std::fabs(glm::dot(worldUp, rotatedXAxis)) * halfScale.x +
        std::fabs(glm::dot(worldUp, rotatedYAxis)) * halfScale.y +
        std::fabs(glm::dot(worldUp, rotatedZAxis)) * halfScale.z;

    float maxHalfExtent = std::max(halfScale.x, std::max(halfScale.y, halfScale.z));
    float extraPadding = std::max(0.2f, maxHalfExtent * 0.18f);
    return sceneObject.transform.position + worldUp * (topOffset + extraPadding);
}

void UIManager::Draw(Scene& scene, Camera& camera, int& selectedIndex,
                     std::function<Mesh*(const std::string&)> getMeshByKey,
                     std::function<std::vector<std::string>()> listMeshKeys,
                     std::function<std::string(const std::string&)> getMeshSourcePath,
                     std::function<Texture*(const std::string&)> getTextureByKey,
                     std::function<std::vector<std::string>()> listTextureKeys,
                     std::function<std::string(const std::string&)> getTextureSourcePath,
                     std::function<void(std::unique_ptr<Message>)> pushMessage)
{
    ImGui::Begin("Scene Hierarchy");
    //
    //Quick Add buttons
    static int cubeCounter = 0;
    static int importedCounter = 0;
    if (ImGui::Button("+ Add Cube"))
    {
        pushMessage(std::make_unique<CreateEntityMessage>(
            "Cube_" + std::to_string(cubeCounter++),
            "Cube"
        ));
    }

    ImGui::SameLine();

    if (ImGui::Button("+ Add Imported"))
    {
        pushMessage(std::make_unique<CreateEntityMessage>(
            "Imported_" + std::to_string(importedCounter++),
            "ImportedOBJ"
        ));
    }


    ImGui::SameLine();

    // ---------------------------
    // DELETE BUTTON
    // ---------------------------
    //FIXME: if nothing is selected we disable delete so we dont explode//
    const bool hasSelection =
        selectedIndex >= 0 && selectedIndex < (int)scene.GetObjects().size();

    ImGui::BeginDisabled(!hasSelection);

    if (ImGui::Button("Delete Selected"))
    {
        auto& selectedObj = scene.GetObjects()[selectedIndex];
        pushMessage(std::make_unique<DeleteEntityMessage>(selectedObj.entity));
        selectedIndex = -1; //safety reset so inspector doesnt point to dead obj//
    }

    ImGui::EndDisabled();


    ImGui::Separator();

    auto& objects = scene.GetObjects();
    for (int i = 0; i < (int)objects.size(); i++)
    {
        bool clicked = ImGui::Selectable(objects[i].name.c_str(), selectedIndex == i);

        if (clicked)
        {
            selectedIndex = i;
        }
    }

    ImGui::End();

    DrawAssetWindow(
        scene,
        selectedIndex,
        getMeshByKey,
        listMeshKeys,
        getMeshSourcePath,
        getTextureByKey,
        listTextureKeys,
        getTextureSourcePath,
        pushMessage);


    ImGui::Begin("Camera");

    //FOV//
    UI::SliderFloatMolten("FOV", &camera.fov, 30.0f, 120.0f);

    ImGui::Separator();

    //Position//
    ImGui::DragFloat3("Position", &camera.position.x, 0.01f);

    //Rotation//
    //Pitch clamp is optional but recommended so it doesn't flip
    ImGui::DragFloat3("Rotation", &camera.rotation.x, 0.10f);

    //Optional clamps so camera doesn't go insane
    if (camera.rotation.x > 89.0f) camera.rotation.x = 89.0f;
    if (camera.rotation.x < -89.0f) camera.rotation.x = -89.0f;

    ImGui::End();

    //If nothing selected dont bother drawing inspector//
    if (selectedIndex != -1 && selectedIndex < (int)scene.GetObjects().size())
    {
        auto& obj = scene.GetObjects()[selectedIndex];

        //Grab my object and add it to the UI, so I can manipulate it //
        ImGui::Begin("Inspector Panel");

        //FIXME: table makes this look 100x cleaner but its annoying sometimes
        //Still better than spaghetti UI so we keep it//
        if (ImGui::BeginTable("##InspectorForm", 2,
            ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_PadOuterX))
        {
            ImGui::TableSetupColumn("##Label", ImGuiTableColumnFlags_WidthFixed, 90.0f);
            ImGui::TableSetupColumn("##Value", ImGuiTableColumnFlags_WidthStretch);

            // ---------------------------
            // Name (Molten input)
            // ---------------------------
            UI::BeginMoltenInput();

            //Fixme: Changing name Context here need to change this later to something else//
            char changedBuffer[62] = "";
            strcpy(changedBuffer , obj.name.c_str());

            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Name");

            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);

            bool changedName = ImGui::InputText("##Name", changedBuffer, sizeof(changedBuffer));
            if(changedName)
            {
                obj.name = changedBuffer;
            }

            UI::EndMoltenInput();

            // ---------------------------
            // Model (Combo + DragDrop)
            // ---------------------------
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Model");

                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);

                std::vector<std::string> keys = listMeshKeys();
                std::vector<const char*> labels;
                labels.reserve(keys.size());

                int currentIndex = 0;
                for (int i = 0; i < (int)keys.size(); i++)
                {
                    labels.push_back(keys[i].c_str());
                    if (keys[i] == obj.meshKey)
                        currentIndex = i;
                }

                if (!labels.empty())
                {
                    if (ImGui::Combo("##Model", &currentIndex, labels.data(), (int)labels.size()))
                    {
                        Mesh* m = getMeshByKey(keys[currentIndex]);
                        if (m != nullptr)
                        {
                            obj.mesh.mesh = m;
                            obj.meshKey = keys[currentIndex];
                        }
                    }
                }
                else
                {
                    ImGui::TextUnformatted("No meshes loaded");
                }

                //Drag target for meshes//
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_KEY"))
                    {
                        const char* droppedKey = (const char*)payload->Data;
                        Mesh* m = getMeshByKey(droppedKey);
                        if (m != nullptr)
                        {
                            obj.mesh.mesh = m;
                            obj.meshKey = droppedKey;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }

            // ---------------------------
            // Texture (Combo + DragDrop)
            // ---------------------------
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Texture");

                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);

                std::vector<std::string> keys = listTextureKeys();
                std::vector<const char*> labels;
                labels.reserve(keys.size());

                int currentIndex = 0;
                for (int i = 0; i < (int)keys.size(); i++)
                {
                    labels.push_back(keys[i].c_str());
                    if (keys[i] == obj.textureKey)
                        currentIndex = i;
                }

                if (!labels.empty())
                {
                    if (ImGui::Combo("##Texture", &currentIndex, labels.data(), (int)labels.size()))
                    {
                        Texture* t = getTextureByKey(keys[currentIndex]);
                        if (t != nullptr)
                        {
                            obj.texture = t;
                            obj.textureKey = keys[currentIndex];
                        }
                    }
                }
                else
                {
                    ImGui::TextUnformatted("No textures loaded");
                }

                //Drag target for textures//
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEX_KEY"))
                    {
                        const char* droppedKey = (const char*)payload->Data;
                        Texture* t = getTextureByKey(droppedKey);
                        if (t != nullptr)
                        {
                            obj.texture = t;
                            obj.textureKey = droppedKey;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }
    // ---------------------------
    // Albedo (Combo + DragDrop)
    // ---------------------------
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Albedo");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);

        std::vector<std::string> keys = listTextureKeys();
        std::vector<const char*> labels;
        labels.reserve(keys.size());

        int currentIndex = 0;
        for (int i = 0; i < (int)keys.size(); i++)
        {
            labels.push_back(keys[i].c_str());
            if (keys[i] == obj.albedoKey)
                currentIndex = i;
        }

        if (!labels.empty())
        {
            if (ImGui::Combo("##Albedo", &currentIndex, labels.data(), (int)labels.size()))
            {
                pushMessage(std::make_unique<SetEntityAlbedoMessage>(obj.entity, keys[currentIndex]));
            }
        }
        else
        {
            ImGui::TextUnformatted("No textures loaded");
        }

        // Drag target for textures
        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEX_KEY"))
            {
                const char* droppedKey = (const char*)payload->Data;
                pushMessage(std::make_unique<SetEntityAlbedoMessage>(obj.entity, std::string(droppedKey)));
            }
            ImGui::EndDragDropTarget();
        }
    }

    // ---------------------------
    // Specular (Combo + DragDrop)
    // ---------------------------
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Specular");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);

        std::vector<std::string> keys = listTextureKeys();
        std::vector<const char*> labels;
        labels.reserve(keys.size());

        int currentIndex = 0;
        for (int i = 0; i < (int)keys.size(); i++)
        {
            labels.push_back(keys[i].c_str());
            if (keys[i] == obj.specularKey)
                currentIndex = i;
        }

        if (!labels.empty())
        {
            if (ImGui::Combo("##Specular", &currentIndex, labels.data(), (int)labels.size()))
            {
                pushMessage(std::make_unique<SetEntitySpecularMessage>(obj.entity, keys[currentIndex]));
            }
        }
        else
        {
            ImGui::TextUnformatted("No textures loaded");
        }

        if (ImGui::BeginDragDropTarget())
        {
            if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("TEX_KEY"))
            {
                const char* droppedKey = (const char*)payload->Data;
                pushMessage(std::make_unique<SetEntitySpecularMessage>(obj.entity, std::string(droppedKey)));
            }
            ImGui::EndDragDropTarget();
        }
    }

    // ---------------------------
    // Shininess
    // ---------------------------
    {
        ImGui::TableNextRow();
        ImGui::TableSetColumnIndex(0);
        ImGui::AlignTextToFramePadding();
        ImGui::TextUnformatted("Shininess");

        ImGui::TableSetColumnIndex(1);
        ImGui::SetNextItemWidth(-FLT_MIN);

        float s = obj.shininess;
        if (ImGui::SliderFloat("##Shininess", &s, 2.0f, 256.0f))
        {
            pushMessage(std::make_unique<SetEntityShininessMessage>(obj.entity, s));
        }
    }

            // ---------------------------
            // Transform (Website-style boxes)
            // ---------------------------
            UI::BeginTransformStyle();

            //This is how we change the position
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Position");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat3("##Position", &obj.transform.position.x, 0.01f);

            //This is how we change the Rotation
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Rotation");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat3("##Rotation", &obj.transform.rotation.x, 0.01f);

            //This is how we scale it yo//
            ImGui::TableNextRow();
            ImGui::TableSetColumnIndex(0);
            ImGui::AlignTextToFramePadding();
            ImGui::TextUnformatted("Scale");
            ImGui::TableSetColumnIndex(1);
            ImGui::SetNextItemWidth(-FLT_MIN);
            ImGui::DragFloat3("##Scale", &obj.transform.scale.x, 0.01f);

            UI::EndTransformStyle();

            ImGui::EndTable();
        }

        ImGui::End();

// ---------------------------
// LIGHTS WINDOW (Phase 3)
// ---------------------------
ImGui::Begin("Lights");

auto& lights = scene.GetLights();
static int selectedLight = 0;

if (ImGui::Button("+ Add Light"))
{
    pushMessage(std::make_unique<AddLightMessage>());
    // keep selection sane
    if (selectedLight < 0) selectedLight = 0;
}
ImGui::SameLine();

bool canDelete = !lights.empty() && selectedLight >= 0 && selectedLight < (int)lights.size();
ImGui::BeginDisabled(!canDelete);
if (ImGui::Button("Delete Light"))
{
    pushMessage(std::make_unique<DeleteLightMessage>(selectedLight));
    if (selectedLight >= (int)lights.size() - 1) selectedLight = (int)lights.size() - 2;
    if (selectedLight < 0) selectedLight = 0;
}
ImGui::EndDisabled();

ImGui::Separator();

if (lights.empty())
{
    ImGui::TextUnformatted("No lights in scene.");
    ImGui::End();
}
else
{
    // Light selector
    std::vector<std::string> lightNames;
    lightNames.reserve(lights.size());
    for (int i = 0; i < (int)lights.size(); ++i)
        lightNames.push_back("Light " + std::to_string(i));

    std::vector<const char*> labels;
    labels.reserve(lightNames.size());
    for (auto& s : lightNames) labels.push_back(s.c_str());

    ImGui::SetNextItemWidth(-FLT_MIN);
    ImGui::Combo("##LightSelect", &selectedLight, labels.data(), (int)labels.size());

    // Edit selected light (local copy -> message)
    if (selectedLight >= 0 && selectedLight < (int)lights.size())
    {
        Light edited = lights[selectedLight];
        bool changed = false;

        changed |= ImGui::DragFloat3("Position", &edited.position.x, 0.05f);
        changed |= ImGui::DragFloat3("Rotation", &edited.rotation.x, 0.5f);
        changed |= ImGui::SliderFloat("Inner Angle", &edited.innerAngle, 1.0f, 60.0f);
        changed |= ImGui::SliderFloat("Outer Angle", &edited.outerAngle, 1.0f, 80.0f);
        if (edited.outerAngle < edited.innerAngle) edited.outerAngle = edited.innerAngle;


        changed |= ImGui::ColorEdit3("Color", &edited.color.x);
        changed |= ImGui::SliderFloat("Intensity", &edited.intensity, 0.0f, 10.0f);
        changed |= ImGui::SliderFloat("Ambient", &edited.ambientStrength, 0.0f, 1.0f);

        if (changed)
        {
            pushMessage(std::make_unique<UpdateLightMessage>(selectedLight, edited));
        }
    }

    ImGui::End();
}

    }
    // ---------------------------
    // Render Settings (ONE window)
    // ---------------------------
    ImGui::Begin("Render Settings");

    static bool mip = true;
    if (ImGui::Checkbox("Use Mipmaps", &mip))
        pushMessage(std::make_unique<SetMipmapsEnabledMessage>(mip));

    static bool shadows = true;
    if (ImGui::Checkbox("Enable Shadows", &shadows))
        pushMessage(std::make_unique<SetShadowsEnabledMessage>(shadows));

    ImGui::Checkbox("Show FPS", &showPerf_);

    ImGui::End();

    ImGui::Begin("Viewport", nullptr,
        ImGuiWindowFlags_NoScrollbar |
        ImGuiWindowFlags_NoScrollWithMouse |
        ImGuiWindowFlags_NoBackground); // important: don’t hide the OpenGL scene

    // Get viewport content rect in SCREEN space
    ImVec2 winPos = ImGui::GetWindowPos();
    ImVec2 crMin  = ImGui::GetWindowContentRegionMin();
    ImVec2 crMax  = ImGui::GetWindowContentRegionMax();
    ImVec2 vpMin  = ImVec2(winPos.x + crMin.x, winPos.y + crMin.y);
    ImVec2 vpMax  = ImVec2(winPos.x + crMax.x, winPos.y + crMax.y);

    viewportMinX_ = vpMin.x;
    viewportMinY_ = vpMin.y;
    viewportMaxX_ = vpMax.x;
    viewportMaxY_ = vpMax.y;
    viewportHovered_ = PointInRect(ImGui::GetMousePos(), vpMin, vpMax);

    // Draw FPS in top-right of viewport content
    if (showPerf_)
    {
        ImGui::SetCursorScreenPos(ImVec2(vpMax.x - 120.0f, vpMin.y + 10.0f));
        ImGui::Text("FPS: %.1f", fps_);
        ImGui::SetCursorScreenPos(ImVec2(vpMax.x - 120.0f, vpMin.y + 30.0f));
        ImGui::Text("MS:  %.2f", ms_);
    }

    ImGui::SetCursorScreenPos(ImVec2(vpMin.x + 12.0f, vpMin.y + 10.0f));
    if (ImGui::SmallButton(showMoveGizmo_ ? "Move Gizmo: ON" : "Move Gizmo: OFF"))
    {
        showMoveGizmo_ = !showMoveGizmo_;
        activeGizmoAxis_ = -1;
        activeGizmoEntityId_ = 0;
    }

    ImGui::SetCursorScreenPos(ImVec2(vpMin.x + 12.0f, vpMin.y + 32.0f));
    ImGui::TextUnformatted("Left drag the colored axis arrows on a selected object");

    ImGui::End();
    DrawViewportGizmo(scene, camera, selectedIndex);

}

void UIManager::DrawAssetWindow(
    Scene& scene,
    int selectedIndex,
    std::function<Mesh*(const std::string&)> getMeshByKey,
    std::function<std::vector<std::string>()> listMeshKeys,
    std::function<std::string(const std::string&)> getMeshSourcePath,
    std::function<Texture*(const std::string&)> getTextureByKey,
    std::function<std::vector<std::string>()> listTextureKeys,
    std::function<std::string(const std::string&)> getTextureSourcePath,
    std::function<void(std::unique_ptr<Message>)> pushMessage)
{
    (void)getMeshByKey;
    (void)getTextureByKey;

    static char objPath[256] = "../assets/models/";
    static char objKey[64]   = "";
    static char texPath[256] = "../assets/";
    static char texKey[64]   = "";
    static int assetSpawnCounter = 0;

    const bool hasSelection = selectedIndex >= 0 && selectedIndex < (int)scene.GetObjects().size();
    SceneObject* selectedObject = hasSelection ? &scene.GetObjects()[selectedIndex] : nullptr;

    std::vector<std::string> meshKeys = listMeshKeys();
    std::vector<std::string> textureKeys = listTextureKeys();
    std::sort(meshKeys.begin(), meshKeys.end());
    std::sort(textureKeys.begin(), textureKeys.end());

    auto importMeshFromBuffer = [&]()
    {
        if (objKey[0] == '\0')
        {
            CopyStringToBuffer(GetFileStem(objPath), objKey, sizeof(objKey));
        }

        pushMessage(std::make_unique<ImportMeshMessage>(
            std::string(objKey),
            std::string(objPath)));
    };

    auto importTextureFromBuffer = [&]()
    {
        if (texKey[0] == '\0')
        {
            CopyStringToBuffer(GetFileStem(texPath), texKey, sizeof(texKey));
        }

        pushMessage(std::make_unique<ImportTextureMessage>(
            std::string(texKey),
            std::string(texPath)));
    };

    ImGui::Begin("Assets");

    ImGui::Text("Loaded: %d meshes | %d textures", (int)meshKeys.size(), (int)textureKeys.size());
    if (hasSelection)
    {
        ImGui::Text("Selected: %s", selectedObject->name.c_str());
    }
    else
    {
        ImGui::TextDisabled("Selected: nothing right now big dog");
    }

    if (ImGui::BeginTabBar("##AssetTabs"))
    {
        if (ImGui::BeginTabItem("Loaded Assets"))
        {
            ImGui::InputTextWithHint(
                "##AssetSearch",
                "Search keys or source paths...",
                assetSearch_,
                sizeof(assetSearch_));
            ImGui::Separator();

            if (ImGui::CollapsingHeader("Meshes", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::BeginTable(
                    "##LoadedMeshes",
                    4,
                    ImGuiTableFlags_Borders |
                    ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_Resizable |
                    ImGuiTableFlags_ScrollY,
                    ImVec2(0.0f, 200.0f)))
                {
                    ImGui::TableSetupColumn("Mesh Key");
                    ImGui::TableSetupColumn("Used", ImGuiTableColumnFlags_WidthFixed, 44.0f);
                    ImGui::TableSetupColumn("Source");
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 130.0f);
                    ImGui::TableHeadersRow();

                    for (const std::string& meshKey : meshKeys)
                    {
                        std::string sourcePath = getMeshSourcePath(meshKey);
                        std::string displaySource = sourcePath.empty() ? "<generated mesh>" : sourcePath;

                        if (!ContainsCaseInsensitive(meshKey, assetSearch_) &&
                            !ContainsCaseInsensitive(displaySource, assetSearch_))
                        {
                            continue;
                        }

                        ImGui::PushID(meshKey.c_str());
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Selectable(meshKey.c_str());
                        if (ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("MESH_KEY", meshKey.c_str(), meshKey.size() + 1);
                            ImGui::Text("Mesh: %s", meshKey.c_str());
                            ImGui::EndDragDropSource();
                        }

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", CountMeshUsage(scene, meshKey));

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(displaySource.c_str());
                        if (ImGui::IsItemHovered() && !sourcePath.empty())
                        {
                            ImGui::SetTooltip("%s", sourcePath.c_str());
                        }

                        ImGui::TableSetColumnIndex(3);
                        ImGui::BeginDisabled(!hasSelection);
                        if (ImGui::SmallButton("Use"))
                        {
                            pushMessage(std::make_unique<SetEntityMeshMessage>(selectedObject->entity, meshKey));
                        }
                        ImGui::EndDisabled();
                        ImGui::SameLine();
                        if (ImGui::SmallButton("+Entity"))
                        {
                            pushMessage(std::make_unique<CreateEntityMessage>(
                                meshKey + "_" + std::to_string(assetSpawnCounter++),
                                meshKey));
                        }

                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }
            }

            if (ImGui::CollapsingHeader("Textures", ImGuiTreeNodeFlags_DefaultOpen))
            {
                if (ImGui::BeginTable(
                    "##LoadedTextures",
                    4,
                    ImGuiTableFlags_Borders |
                    ImGuiTableFlags_RowBg |
                    ImGuiTableFlags_Resizable |
                    ImGuiTableFlags_ScrollY,
                    ImVec2(0.0f, 200.0f)))
                {
                    ImGui::TableSetupColumn("Texture Key");
                    ImGui::TableSetupColumn("Used", ImGuiTableColumnFlags_WidthFixed, 44.0f);
                    ImGui::TableSetupColumn("Source");
                    ImGui::TableSetupColumn("Action", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                    ImGui::TableHeadersRow();

                    for (const std::string& textureKey : textureKeys)
                    {
                        std::string sourcePath = getTextureSourcePath(textureKey);
                        std::string displaySource = sourcePath.empty() ? "<generated texture>" : sourcePath;

                        if (!ContainsCaseInsensitive(textureKey, assetSearch_) &&
                            !ContainsCaseInsensitive(displaySource, assetSearch_))
                        {
                            continue;
                        }

                        ImGui::PushID(textureKey.c_str());
                        ImGui::TableNextRow();

                        ImGui::TableSetColumnIndex(0);
                        ImGui::Selectable(textureKey.c_str());
                        if (ImGui::BeginDragDropSource())
                        {
                            ImGui::SetDragDropPayload("TEX_KEY", textureKey.c_str(), textureKey.size() + 1);
                            ImGui::Text("Texture: %s", textureKey.c_str());
                            ImGui::EndDragDropSource();
                        }

                        ImGui::TableSetColumnIndex(1);
                        ImGui::Text("%d", CountTextureUsage(scene, textureKey));

                        ImGui::TableSetColumnIndex(2);
                        ImGui::TextUnformatted(displaySource.c_str());
                        if (ImGui::IsItemHovered() && !sourcePath.empty())
                        {
                            ImGui::SetTooltip("%s", sourcePath.c_str());
                        }

                        ImGui::TableSetColumnIndex(3);
                        ImGui::BeginDisabled(!hasSelection);
                        if (ImGui::SmallButton("Use"))
                        {
                            pushMessage(std::make_unique<SetEntityTextureMessage>(selectedObject->entity, textureKey));
                        }
                        ImGui::EndDisabled();

                        ImGui::PopID();
                    }

                    ImGui::EndTable();
                }
            }

            ImGui::EndTabItem();
        }

        if (ImGui::BeginTabItem("Project Browser"))
        {
            ImGui::TextUnformatted("Quick Import");
            if (ImGui::BeginTable("##QuickImportTable", 2, ImGuiTableFlags_SizingStretchSame))
            {
                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::TextUnformatted("OBJ Path");
                ImGui::InputText("##ObjPath", objPath, sizeof(objPath));
                ImGui::TextUnformatted("OBJ Key");
                ImGui::InputText("##ObjKey", objKey, sizeof(objKey));
                if (ImGui::Button("Import OBJ"))
                {
                    importMeshFromBuffer();
                }

                ImGui::TableSetColumnIndex(1);
                ImGui::TextUnformatted("Texture Path");
                ImGui::InputText("##TexPath", texPath, sizeof(texPath));
                ImGui::TextUnformatted("Texture Key");
                ImGui::InputText("##TexKey", texKey, sizeof(texKey));
                if (ImGui::Button("Import Texture"))
                {
                    importTextureFromBuffer();
                }

                ImGui::EndTable();
            }

            ImGui::Separator();
            ImGui::InputTextWithHint(
                "##ProjectAssetSearch",
                "Search files inside ../assets...",
                projectAssetSearch_,
                sizeof(projectAssetSearch_));

            fs::path projectRoot = "../assets";
            std::vector<fs::path> projectFiles;

            if (fs::exists(projectRoot))
            {
                try
                {
                    for (const auto& entry : fs::recursive_directory_iterator(
                        projectRoot,
                        fs::directory_options::skip_permission_denied))
                    {
                        if (!entry.is_regular_file())
                        {
                            continue;
                        }

                        if (!IsMeshAssetPath(entry.path()) && !IsTextureAssetPath(entry.path()))
                        {
                            continue;
                        }

                        projectFiles.push_back(entry.path());
                    }
                }
                catch (const fs::filesystem_error&)
                {
                    //If the file browser throws a fit we just skip the drama and keep the editor alive//
                }
            }

            std::sort(
                projectFiles.begin(),
                projectFiles.end(),
                [](const fs::path& leftPath, const fs::path& rightPath)
                {
                    return leftPath.generic_string() < rightPath.generic_string();
                });

            if (ImGui::BeginTable(
                "##ProjectAssetTable",
                3,
                ImGuiTableFlags_Borders |
                ImGuiTableFlags_RowBg |
                ImGuiTableFlags_Resizable |
                ImGuiTableFlags_ScrollY,
                ImVec2(0.0f, 300.0f)))
            {
                ImGui::TableSetupColumn("File");
                ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 64.0f);
                ImGui::TableSetupColumn("Quick Action", ImGuiTableColumnFlags_WidthFixed, 110.0f);
                ImGui::TableHeadersRow();

                for (const fs::path& assetPath : projectFiles)
                {
                    std::string relativePath;
                    try
                    {
                        relativePath = fs::relative(assetPath, projectRoot).generic_string();
                    }
                    catch (const fs::filesystem_error&)
                    {
                        relativePath = assetPath.generic_string();
                    }

                    if (!ContainsCaseInsensitive(relativePath, projectAssetSearch_))
                    {
                        continue;
                    }

                    bool isMeshFile = IsMeshAssetPath(assetPath);
                    bool isTextureFile = IsTextureAssetPath(assetPath);
                    std::string assetType = isMeshFile ? "Mesh" : "Texture";
                    std::string fullPath = assetPath.generic_string();
                    std::string stemKey = assetPath.stem().string();

                    ImGui::PushID(relativePath.c_str());
                    ImGui::TableNextRow();

                    ImGui::TableSetColumnIndex(0);
                    bool clicked = ImGui::Selectable(
                        relativePath.c_str(),
                        false,
                        ImGuiSelectableFlags_AllowDoubleClick);

                    if (clicked)
                    {
                        if (isMeshFile)
                        {
                            CopyStringToBuffer(fullPath, objPath, sizeof(objPath));
                            CopyStringToBuffer(stemKey, objKey, sizeof(objKey));
                            if (ImGui::IsMouseDoubleClicked(0))
                            {
                                importMeshFromBuffer();
                            }
                        }
                        else if (isTextureFile)
                        {
                            CopyStringToBuffer(fullPath, texPath, sizeof(texPath));
                            CopyStringToBuffer(stemKey, texKey, sizeof(texKey));
                            if (ImGui::IsMouseDoubleClicked(0))
                            {
                                importTextureFromBuffer();
                            }
                        }
                    }

                    ImGui::TableSetColumnIndex(1);
                    ImGui::TextUnformatted(assetType.c_str());

                    ImGui::TableSetColumnIndex(2);
                    if (ImGui::SmallButton("Queue"))
                    {
                        if (isMeshFile)
                        {
                            CopyStringToBuffer(fullPath, objPath, sizeof(objPath));
                            CopyStringToBuffer(stemKey, objKey, sizeof(objKey));
                        }
                        else if (isTextureFile)
                        {
                            CopyStringToBuffer(fullPath, texPath, sizeof(texPath));
                            CopyStringToBuffer(stemKey, texKey, sizeof(texKey));
                        }
                    }

                    ImGui::SameLine();
                    if (ImGui::SmallButton("Import"))
                    {
                        if (isMeshFile)
                        {
                            CopyStringToBuffer(fullPath, objPath, sizeof(objPath));
                            CopyStringToBuffer(stemKey, objKey, sizeof(objKey));
                            importMeshFromBuffer();
                        }
                        else if (isTextureFile)
                        {
                            CopyStringToBuffer(fullPath, texPath, sizeof(texPath));
                            CopyStringToBuffer(stemKey, texKey, sizeof(texKey));
                            importTextureFromBuffer();
                        }
                    }

                    ImGui::PopID();
                }

                ImGui::EndTable();
            }

            ImGui::TextDisabled("Tip: single click queues the path, double click imports it immediately.");
            ImGui::EndTabItem();
        }

        ImGui::EndTabBar();
    }

    ImGui::End();
}

void UIManager::DrawViewportGizmo(Scene& scene, Camera& camera, int selectedIndex)
{
    if (!showMoveGizmo_)
    {
        activeGizmoAxis_ = -1;
        activeGizmoEntityId_ = 0;
        return;
    }

    if (selectedIndex < 0 || selectedIndex >= (int)scene.GetObjects().size())
    {
        if (!ImGui::IsMouseDown(0))
        {
            activeGizmoAxis_ = -1;
            activeGizmoEntityId_ = 0;
        }
        return;
    }

    auto& selectedObject = scene.GetObjects()[selectedIndex];
    glm::vec3 gizmoAnchorWorld = GetSceneGizmoAnchor(selectedObject);
    ImVec2 viewportMin(viewportMinX_, viewportMinY_);
    ImVec2 viewportMax(viewportMaxX_, viewportMaxY_);
    ImVec2 screenSize = ImGui::GetIO().DisplaySize;

    if (viewportMax.x <= viewportMin.x || viewportMax.y <= viewportMin.y)
    {
        return;
    }

    ImVec2 originScreen;
    if (!ProjectWorldToScreen(gizmoAnchorWorld, camera, screenSize, originScreen))
    {
        if (!ImGui::IsMouseDown(0))
        {
            activeGizmoAxis_ = -1;
            activeGizmoEntityId_ = 0;
        }
        return;
    }

    if (!PointInRect(originScreen, viewportMin, viewportMax))
    {
        if (!ImGui::IsMouseDown(0))
        {
            activeGizmoAxis_ = -1;
            activeGizmoEntityId_ = 0;
        }
        return;
    }

    std::array<glm::vec3, 3> axisDirections =
    {
        glm::vec3(1.0f, 0.0f, 0.0f),
        glm::vec3(0.0f, 1.0f, 0.0f),
        glm::vec3(0.0f, 0.0f, 1.0f)
    };

    std::array<ImVec2, 3> axisEndPoints{};
    std::array<bool, 3> axisVisible{ false, false, false };
    std::array<float, 3> axisPixelsPerUnit{ 1.0f, 1.0f, 1.0f };
    std::array<ImVec2, 3> axisScreenDirections{};

    float cameraDistance = glm::length(camera.position - gizmoAnchorWorld);
    float handleWorldLength = std::clamp(cameraDistance * 0.07f, 0.30f, 1.20f);
    ImVec2 mousePosition = ImGui::GetMousePos();

    int hoveredAxis = -1;
    float bestHoverDistance = 9999.0f;

    for (int axisIndex = 0; axisIndex < 3; ++axisIndex)
    {
        glm::vec3 worldEndPoint = gizmoAnchorWorld + axisDirections[axisIndex] * handleWorldLength;
        ImVec2 axisEndPoint;
        if (!ProjectWorldToScreen(worldEndPoint, camera, screenSize, axisEndPoint))
        {
            continue;
        }

        float axisScreenLength = std::sqrt(
            (axisEndPoint.x - originScreen.x) * (axisEndPoint.x - originScreen.x) +
            (axisEndPoint.y - originScreen.y) * (axisEndPoint.y - originScreen.y));

        if (axisScreenLength < 18.0f)
        {
            continue;
        }

        axisVisible[axisIndex] = true;
        axisEndPoints[axisIndex] = axisEndPoint;
        axisPixelsPerUnit[axisIndex] = axisScreenLength / handleWorldLength;
        axisScreenDirections[axisIndex] = ImVec2(
            (axisEndPoint.x - originScreen.x) / axisScreenLength,
            (axisEndPoint.y - originScreen.y) / axisScreenLength);

        if (!viewportHovered_ || !PointInRect(mousePosition, viewportMin, viewportMax))
        {
            continue;
        }

        float hoverDistance = DistanceToLineSegment(mousePosition, originScreen, axisEndPoint);
        float tipDistance = std::sqrt(
            (mousePosition.x - axisEndPoint.x) * (mousePosition.x - axisEndPoint.x) +
            (mousePosition.y - axisEndPoint.y) * (mousePosition.y - axisEndPoint.y));
        float pickDistance = std::min(hoverDistance, tipDistance);

        if (pickDistance < 24.0f && pickDistance < bestHoverDistance)
        {
            bestHoverDistance = pickDistance;
            hoveredAxis = axisIndex;
        }
    }

    if (activeGizmoAxis_ != -1 && activeGizmoEntityId_ != selectedObject.entity.Id)
    {
        activeGizmoAxis_ = -1;
        activeGizmoEntityId_ = 0;
    }

    if (activeGizmoAxis_ == -1 &&
        hoveredAxis != -1 &&
        ImGui::IsMouseClicked(0) &&
        viewportHovered_)
    {
        activeGizmoAxis_ = hoveredAxis;
        activeGizmoEntityId_ = selectedObject.entity.Id;
        gizmoStartMouseX_ = mousePosition.x;
        gizmoStartMouseY_ = mousePosition.y;
        gizmoStartPosition_ = selectedObject.transform.position;
        gizmoAxisDirection_ = axisDirections[hoveredAxis];
        gizmoPixelsPerUnit_ = axisPixelsPerUnit[hoveredAxis];
        gizmoAxisScreenX_ = axisScreenDirections[hoveredAxis].x;
        gizmoAxisScreenY_ = axisScreenDirections[hoveredAxis].y;
    }

    if (activeGizmoAxis_ != -1 && activeGizmoEntityId_ == selectedObject.entity.Id)
    {
        if (ImGui::IsMouseDown(0))
        {
            float mouseDeltaAlongAxis =
                (mousePosition.x - gizmoStartMouseX_) * gizmoAxisScreenX_ +
                (mousePosition.y - gizmoStartMouseY_) * gizmoAxisScreenY_;

            float worldUnitsMoved = mouseDeltaAlongAxis / std::max(gizmoPixelsPerUnit_, 1.0f);
            if (ImGui::GetIO().KeyShift)
            {
                //Little snap mode because sometimes precision matters and sometimes the mouse is just being rude//
                worldUnitsMoved = std::round(worldUnitsMoved * 4.0f) / 4.0f;
            }

            selectedObject.transform.position = gizmoStartPosition_ + gizmoAxisDirection_ * worldUnitsMoved;
        }
        else
        {
            activeGizmoAxis_ = -1;
            activeGizmoEntityId_ = 0;
        }
    }

    if (hoveredAxis != -1 || activeGizmoAxis_ != -1)
    {
        ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll);
    }
}

void UIManager::DrawPlayHUD(const SplineShooterGame& game, std::function<void(std::unique_ptr<Message>)> pushMessage)
{
    GameUI::DrawPlayHUD(
        game,
        fps_,
        ms_,
        showPerf_,
        [pushMessage]()
        {
            pushMessage(std::make_unique<StartGameMessage>());
        },
        [pushMessage]()
        {
            pushMessage(std::make_unique<StopGameMessage>());
        });
}

void UIManager::LoadSaveSceneUI(std::function<void(std::unique_ptr<Message>)> pushMessage)
{
    static char scenePath[256] = "../scenes/test.scene.json";
    static bool openSave = false;
    static bool openLoad = false;

    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Save Scene...")) openSave = true;
            if (ImGui::MenuItem("Load Scene...")) openLoad = true;
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Game"))
        {
            if (ImGui::MenuItem("Play"))pushMessage(std::make_unique<StartGameMessage>());
            if (ImGui::MenuItem("Stop"))pushMessage(std::make_unique<StopGameMessage>());
            ImGui::EndMenu();
        }

        ImGui::EndMainMenuBar();
    }

    if (openSave) ImGui::OpenPopup("Save Scene");
    if (ImGui::BeginPopupModal("Save Scene", &openSave, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Path", scenePath, sizeof(scenePath));
        if (ImGui::Button("Save"))
        {
            pushMessage(std::make_unique<SaveSceneMessage>(std::string(scenePath)));
            openSave = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            openSave = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

    if (openLoad) ImGui::OpenPopup("Load Scene");
    if (ImGui::BeginPopupModal("Load Scene", &openLoad, ImGuiWindowFlags_AlwaysAutoResize))
    {
        ImGui::InputText("Path", scenePath, sizeof(scenePath));
        if (ImGui::Button("Load"))
        {
            pushMessage(std::make_unique<LoadSceneMessage>(std::string(scenePath)));
            openLoad = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::SameLine();
        if (ImGui::Button("Cancel"))
        {
            openLoad = false;
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }

}

