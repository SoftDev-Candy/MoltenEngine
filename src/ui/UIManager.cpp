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

void UIManager::Draw(Scene &scene, Camera &camera, int& selectedIndex,
                     std::function<Entity(const std::string &)> createCube,
                     std::function<Entity(const std::string&)> createImported,

                     //Meshes//
                     std::function<Mesh*(const std::string&)> getMeshByKey,
                     std::function<std::vector<std::string>()> listMeshKeys,
                     std::function<bool(const std::string&, const std::string&)> importObjMesh,

                     //Textures//
                     std::function<Texture*(const std::string&)> getTextureByKey,
                     std::function<std::vector<std::string>()> listTextureKeys,
                     std::function<bool(const std::string&, const std::string&)> importTexture)
{
    ImGui::Begin("Scene Hierarchy");

    // ---------------------------
    // QUICK ADD BUTTONS (so the teacher can SEE it, not just "trust me bro")
    // ---------------------------
    static int cubeCounter = 0;
    static int importedCounter = 0;

    if (ImGui::Button("+ Add Cube"))
    {
        //TODO: later we make a proper naming system / entity factory etc
        //Right now this is just proof of life//
        createCube("Cube_" + std::to_string(cubeCounter++));
    }

    ImGui::SameLine();

    if (ImGui::Button("+ Add Imported"))
    {
        //FIXME: this name is cringe but it works so we move//
        createImported("Imported_" + std::to_string(importedCounter++));
    }

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
        importObjMesh(objKey, objPath);
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

        importTexture(texKey, texPath);
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

    ImGui::Begin("Camera FOV");
    UI::SliderFloatMolten("FOV", &camera.fov, 30.0f, 120.0f);
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
    }
}
