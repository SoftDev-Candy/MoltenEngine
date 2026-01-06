//
// Created by Candy on 12/18/2025.
//  UIManager: draws the editor UI panels + lets you spawn stuff and edit it
//

#include "UIManager.hpp"

#include "EditorWidgets.hpp"
#include "imgui_impl_opengl3.h"
#include <cstring> // strcpy
#include <string>  // std::to_string

void UIManager::Draw(Scene &scene, Camera &camera, int& selectedIndex,
                     std::function<Entity(const std::string &)> createCube,
                     std::function<Entity(const std::string&)> createImported,
                     std::function<Mesh*(const std::string&)> getMeshByKey)
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
            // OPTIONAL: Model selection (Cube / ImportedOBJ)
            // ---------------------------
            {
                //This is the engine part: we compare pointers, not strings everywhere//
                //We ask engine for mesh pointers via callback (so UI is not tightly coupled to MeshManager)//
                Mesh* cubeMesh = getMeshByKey("Cube");
                Mesh* importedMesh = getMeshByKey("ImportedOBJ");

                //TODO: later this becomes a dynamic asset list not hardcoded 2 options//
                const char* models[] = { "Cube", "ImportedOBJ" };
                int modelIndex = 0;

                //If this object is using imported mesh -> index 1, else default to cube//
                if (obj.mesh.mesh == importedMesh)
                    modelIndex = 1;

                ImGui::TableNextRow();
                ImGui::TableSetColumnIndex(0);
                ImGui::AlignTextToFramePadding();
                ImGui::TextUnformatted("Model");

                ImGui::TableSetColumnIndex(1);
                ImGui::SetNextItemWidth(-FLT_MIN);

                if (ImGui::Combo("##Model", &modelIndex, models, IM_ARRAYSIZE(models)))
                {
                    //Swap mesh pointer based on dropdown selection//
                    if (modelIndex == 0)
                        obj.mesh.mesh = cubeMesh;
                    else
                        obj.mesh.mesh = importedMesh;

                    //FIXME: if mesh is nullptr it will just not render (safe)
                    //But later we should show warning in UI//
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
