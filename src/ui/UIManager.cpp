//
// Created by Candy on 12/18/2025.
//  UIManager: draws the editor UI panels + lets you spawn stuff and edit it
//

#include "UIManager.hpp"

#include "EditorWidgets.hpp"
#include "imgui_impl_opengl3.h"
#include <cstring> // strcpy
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

void UIManager::Draw(Scene& scene, Camera& camera, int& selectedIndex,
                     std::function<Mesh*(const std::string&)> getMeshByKey,
                     std::function<std::vector<std::string>()> listMeshKeys,
                     std::function<Texture*(const std::string&)> getTextureByKey,
                     std::function<std::vector<std::string>()> listTextureKeys,
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

    // ---------------------------
    // ASSETS WINDOW (Meshes + Textures + Import)
    //Teacher will love this because its "engine-y"//
    // ---------------------------
    ImGui::Begin("Assets");

    //OBJ import (path + key)//
    static char objPath[256] = "../assets/models/";
    static char objKey[64]   = "";

    ImGui::TextUnformatted("OBJ Import");
    ImGui::InputText("##ObjPath", objPath, sizeof(objPath));
    ImGui::InputText("##ObjKey",  objKey,  sizeof(objKey));
    ImGui::SameLine();
    if (ImGui::Button("Import OBJ"))
    {
        //If no key typed just use file name//
        if (objKey[0] == '\0')
        {
            std::string stem = GetFileStem(objPath);
            std::strncpy(objKey, stem.c_str(), sizeof(objKey));
            objKey[sizeof(objKey) - 1] = '\0';
        }

        //FIXME: import function belongs to engine not UI, so we call callback//
        pushMessage(std::make_unique<ImportMeshMessage>(
            std::string(objKey),
            std::string(objPath)
        ));
    }

    ImGui::Separator();

    //Texture import (path + key)//
    static char texPath[256] = "../assets/";
    static char texKey[64]   = "";

    ImGui::TextUnformatted("Texture Import");
    ImGui::InputText("##TexPath", texPath, sizeof(texPath));
    ImGui::InputText("##TexKey",  texKey,  sizeof(texKey));
    ImGui::SameLine();
    if (ImGui::Button("Import Texture"))
    {
        //If no key typed just use file name//
        if (texKey[0] == '\0')
        {
            std::string stem = GetFileStem(texPath);
            std::strncpy(texKey, stem.c_str(), sizeof(texKey));
            texKey[sizeof(texKey) - 1] = '\0';
        }

        pushMessage(std::make_unique<ImportTextureMessage>(
            std::string(texKey),
            std::string(texPath)
        ));
    }

    ImGui::Separator();

    //Mesh list (drag to Inspector -> Model)//
    ImGui::TextUnformatted("Meshes (drag onto Model)");
    {
        std::vector<std::string> keys = listMeshKeys();
        for (const std::string& k : keys)
        {
            ImGui::Selectable(k.c_str());

            //Drag source//
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("MESH_KEY", k.c_str(), k.size() + 1);
                ImGui::Text("Mesh: %s", k.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

    ImGui::Separator();

    //Texture list (drag to Inspector -> Texture)//
    ImGui::TextUnformatted("Textures (drag onto Texture)");
    {
        std::vector<std::string> keys = listTextureKeys();
        for (const std::string& k : keys)
        {
            ImGui::Selectable(k.c_str());

            //Drag source//
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("TEX_KEY", k.c_str(), k.size() + 1);
                ImGui::Text("Texture: %s", k.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }


    ImGui::End();

    //TEXTURE END's HERE


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

    // Draw FPS in top-right of viewport content
    if (showPerf_)
    {
        ImGui::SetCursorScreenPos(ImVec2(vpMax.x - 120.0f, vpMin.y + 10.0f));
        ImGui::Text("FPS: %.1f", fps_);
        ImGui::SetCursorScreenPos(ImVec2(vpMax.x - 120.0f, vpMin.y + 30.0f));
        ImGui::Text("MS:  %.2f", ms_);
    }

    ImGui::End();


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

