//
// Created by Candy on 12/17/2025.
//

#include "EditorWidgets.hpp"

#include "imgui_internal.h"
#include "../imgui/imgui.h"

namespace UI {

static ImVec4 Molten1() { return ImVec4(0.96f, 0.48f, 0.28f, 1.00f); }
static ImVec4 Molten2() { return ImVec4(0.88f, 0.33f, 0.20f, 1.00f); }

void BeginDockspaceAndTopBar()
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);
    ImGui::SetNextWindowViewport(vp->ID);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDocking | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize  | ImGuiWindowFlags_NoMove     | ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoNavFocus| ImGuiWindowFlags_NoBackground;

    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0)); // fully transparent root

    ImGui::Begin("DockspaceRoot", nullptr, flags);

    ImGui::PopStyleColor();
    ImGui::PopStyleVar(2);

    ImGuiID dockspace_id = ImGui::GetID("MainDockspace");
    ImGui::DockSpace(dockspace_id, ImVec2(0,0), ImGuiDockNodeFlags_PassthruCentralNode);

    // Glass top bar (fake "glass frame")
    const float barH = 42.0f;
    ImDrawList* dl = ImGui::GetWindowDrawList();
    ImVec2 p0 = vp->Pos;
    ImVec2 p1 = ImVec2(vp->Pos.x + vp->Size.x, vp->Pos.y + barH);

    ImU32 topA = ImGui::GetColorU32(ImVec4(1,1,1,0.10f));
    ImU32 topB = ImGui::GetColorU32(ImVec4(0,0,0,0.35f));
    dl->AddRectFilledMultiColor(p0, p1, topA, topA, topB, topB); // subtle vertical gradient
    dl->AddLine(ImVec2(p0.x, p1.y), ImVec2(p1.x, p1.y), ImGui::GetColorU32(ImVec4(1,1,1,0.10f)));

    ImGui::End();
}

bool SliderFloatMolten(const char* label, float* v, float v_min, float v_max)
{
    // Build the slider first (so ImGui computes layout + grab)
    bool changed = ImGui::SliderFloat(label, v, v_min, v_max);

    // Lava palette
    const ImVec4 lava1 = ImVec4(0.957f, 0.486f, 0.282f, 1.00f); // #F47C48
    const ImVec4 lava2 = ImVec4(0.882f, 0.337f, 0.200f, 1.00f); // #E15633
    const ImVec4 lava3 = ImVec4(0.706f, 0.325f, 0.133f, 1.00f); // #B45322
    const ImVec4 hot1  = ImVec4(1.000f, 0.580f, 0.400f, 1.00f); // #FF9466

    // Normalized t
    float t = (*v - v_min) / (v_max - v_min);
    if (t < 0.0f) t = 0.0f;
    if (t > 1.0f) t = 1.0f;

    // Item rect includes label area sometimes; we want the frame area.
    ImVec2 itemMin = ImGui::GetItemRectMin();
    ImVec2 itemMax = ImGui::GetItemRectMax();

    // Approx frame rect: keep full width but only the typical frame height.
    // This avoids painting behind the label area.
    const float frameH = ImGui::GetFrameHeight();
    ImVec2 frameMin = itemMin;
    ImVec2 frameMax = ImVec2(itemMax.x, itemMin.y + frameH);

    // Padding inside the frame so we don't cover borders
    const float pad = 2.0f;
    ImVec2 a = ImVec2(frameMin.x + pad, frameMin.y + pad);
    ImVec2 b = ImVec2(frameMax.x - pad, frameMax.y - pad);

    float fillW = (b.x - a.x) * t;
    if (fillW < 0.0f) fillW = 0.0f;
    if (fillW > (b.x - a.x)) fillW = (b.x - a.x);

    ImDrawList* dl = ImGui::GetWindowDrawList();

    // Two-stage molten fill: magma -> red -> hot core near grab
    ImVec2 mid = ImVec2(a.x + fillW * 0.70f, b.y);
    ImVec2 end = ImVec2(a.x + fillW, b.y);

    if (fillW > 0.0f)
    {
        // Segment 1: lava3 -> lava2
        dl->AddRectFilledMultiColor(
            a, mid,
            ImGui::GetColorU32(lava3),
            ImGui::GetColorU32(lava2),
            ImGui::GetColorU32(lava2),
            ImGui::GetColorU32(lava3)
        );

        // Segment 2: lava2 -> hot1 (near knob)
        if (end.x > mid.x)
        {
            dl->AddRectFilledMultiColor(
                ImVec2(mid.x, a.y), end,
                ImGui::GetColorU32(lava2),
                ImGui::GetColorU32(hot1),
                ImGui::GetColorU32(hot1),
                ImGui::GetColorU32(lava2)
            );
        }
    }

    return changed;
}


void BeginMoltenInput()
{
    ImVec4 molten = Molten1();
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.02f, 0.02f, 0.02f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.05f, 0.05f, 0.05f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(0.07f, 0.07f, 0.07f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_Border,         ImVec4(molten.x, molten.y, molten.z, 0.45f));
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(molten.x, molten.y, molten.z, 1.00f));
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
}

void EndMoltenInput()
{
    ImGui::PopStyleVar();
    ImGui::PopStyleColor(5);
}


    void BeginTransformStyle()
{
    // Website lava palette
    const ImVec4 lava1 = ImVec4(0.957f, 0.486f, 0.282f, 1.00f); // #F47C48
    const ImVec4 lava2 = ImVec4(0.882f, 0.337f, 0.200f, 1.00f); // #E15633
    const ImVec4 lava3 = ImVec4(0.706f, 0.325f, 0.133f, 1.00f); // #B45322

    // --- TWEAK KNOBS ---
    const float hoverA  = 0.55f; // hover strength (0.35..0.85)
    const float activeA = 0.80f; // active strength (0.50..1.00)
    const float borderA = 0.65f; // border intensity (0.25..0.90)
    const float rounding = 10.0f; // box rounding
    // -------------------

    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, rounding);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);

    // Black box + white text
    ImGui::PushStyleColor(ImGuiCol_Text,           ImVec4(1,1,1,1));
    ImGui::PushStyleColor(ImGuiCol_FrameBg,        ImVec4(0.02f, 0.02f, 0.02f, 1.00f));

    // Hover = slightly darker orange (lava3)
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(lava3.x, lava3.y, lava3.z, hoverA));

    // Active = hotter (lava2)
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive,  ImVec4(lava2.x, lava2.y, lava2.z, activeA));

    // Border = lava rim
    ImGui::PushStyleColor(ImGuiCol_Border,         ImVec4(lava1.x, lava1.y, lava1.z, borderA));
}

    void EndTransformStyle()
{
    ImGui::PopStyleColor(5);
    ImGui::PopStyleVar(2);
}

    // Optional helper: makes the VALUE text go orange while active/dragging
    bool InputTextLava(const char* id, char* buf, size_t bufSize)
{
    const ImVec4 lava1 = ImVec4(0.957f, 0.486f, 0.282f, 1.00f);
    ImGuiID wid = ImGui::GetID(id);
    bool isActive = (ImGui::GetActiveID() == wid);

    if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, lava1);
    bool changed = ImGui::InputText(id, buf, bufSize);
    if (isActive) ImGui::PopStyleColor();

    return changed;
}

    bool DragFloat3Lava(const char* id, float* v, float speed)
{
    const ImVec4 lava1 = ImVec4(0.957f, 0.486f, 0.282f, 1.00f);
    ImGuiID wid = ImGui::GetID(id);
    bool isActive = (ImGui::GetActiveID() == wid);

    if (isActive) ImGui::PushStyleColor(ImGuiCol_Text, lava1);
    bool changed = ImGui::DragFloat3(id, v, speed);
    if (isActive) ImGui::PopStyleColor();

    return changed;
}


} // namespace UI
