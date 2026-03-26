#include "GameUI.hpp"

#include "Game.hpp"
#include <imgui.h>

namespace GameUI
{
    static void DrawCenteredText(const char* text)
    {
        float textWidth = ImGui::CalcTextSize(text).x;
        float centeredX = (ImGui::GetWindowSize().x - textWidth) * 0.5f;
        if (centeredX > 0.0f)
        {
            ImGui::SetCursorPosX(centeredX);
        }
        ImGui::TextUnformatted(text);
    }

    static bool DrawCenteredButton(const char* label, float width)
    {
        float centeredX = (ImGui::GetWindowSize().x - width) * 0.5f;
        if (centeredX > 0.0f)
        {
            ImGui::SetCursorPosX(centeredX);
        }
        return ImGui::Button(label, ImVec2(width, 0.0f));
    }

    void DrawPlayHUD(
        const SplineShooterGame& game,
        float framesPerSecond,
        float millisecondsPerFrame,
        bool showPerformanceStats,
        const std::function<void()>& onStartRequested,
        const std::function<void()>& onStopRequested)
    {
        ImGuiViewport* mainViewport = ImGui::GetMainViewport();

        ImGuiWindowFlags hudWindowFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoFocusOnAppearing |
            ImGuiWindowFlags_NoNav;

        if (game.GetState() == SplineShooterGame::GameState::Playing)
        {
            //Tiny cockpit HUD lives here now so gameplay code can stop cosplaying as a UI file//
            ImGui::SetNextWindowViewport(mainViewport->ID);
            ImGui::SetNextWindowPos(
                ImVec2(mainViewport->WorkPos.x + 16.0f, mainViewport->WorkPos.y + 16.0f),
                ImGuiCond_Always);
            ImGui::SetNextWindowBgAlpha(0.42f);

            if (ImGui::Begin("PlayHUD##SplineShooter", nullptr, hudWindowFlags))
            {
                const char* controlModeLabel =
                    (game.GetControlMode() == SplineShooterGame::ControlMode::FreeFly)
                        ? "FreeFly"
                        : "SplineFollow";

                ImGui::TextUnformatted("SPLINE SHOOTER");
                ImGui::Separator();
                ImGui::Text("State:  %s", game.GetStateLabel());
                ImGui::Text("Health: %d", game.GetHealth());
                ImGui::Text("Score:  %d", game.GetScore());
                ImGui::Text("Mode:   %s", controlModeLabel);
                ImGui::TextUnformatted("A/D strafe  Q/E up-down  SPACE shoot");
                ImGui::TextUnformatted("F swaps control mode  ESC quits run");

                if (showPerformanceStats)
                {
                    //FPS numbers stay in the corner so the menu doesnt turn into a science fair board//
                    ImGui::Separator();
                    ImGui::Text("FPS: %.1f", framesPerSecond);
                    ImGui::Text("MS:  %.2f", millisecondsPerFrame);
                }

                ImGui::Separator();
                if (ImGui::Button("Quit Game") && onStopRequested)
                {
                    onStopRequested();
                }
            }
            ImGui::End();
            return;
        }

        ImGuiWindowFlags menuWindowFlags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoDocking |
            ImGuiWindowFlags_AlwaysAutoResize |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoNav;

        ImGui::SetNextWindowViewport(mainViewport->ID);
        ImGui::SetNextWindowPos(
            ImVec2(mainViewport->WorkPos.x + mainViewport->WorkSize.x * 0.5f,
                   mainViewport->WorkPos.y + mainViewport->WorkSize.y * 0.5f),
            ImGuiCond_Always,
            ImVec2(0.5f, 0.5f));
        ImGui::SetNextWindowSize(ImVec2(360.0f, 0.0f), ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.88f);

        if (ImGui::Begin("SplineShooterMenu##MainMenu", nullptr, menuWindowFlags))
        {
            const char* titleText = "SPLINE SHOOTER";
            const char* subtitleText = "Ride the spline and try not to explode.";
            const char* startButtonLabel = "Start Game";

            if (game.GetState() == SplineShooterGame::GameState::Win)
            {
                titleText = "YOU WIN";
                subtitleText = "You reached the end of the scene. Nice one.";
                startButtonLabel = "Play Again";
            }
            else if (game.GetState() == SplineShooterGame::GameState::Lose)
            {
                titleText = "YOU LOSE";
                subtitleText = "Those rocks were not interested in diplomacy.";
                startButtonLabel = "Try Again";
            }

            DrawCenteredText(titleText);
            ImGui::Separator();
            DrawCenteredText(subtitleText);
            ImGui::Spacing();
            ImGui::Text("Health: %d", game.GetHealth());
            ImGui::Text("Score:  %d", game.GetScore());

            if (game.GetState() == SplineShooterGame::GameState::Start)
            {
                ImGui::Spacing();
                ImGui::TextWrapped("Need at least 4 objects named SplinePoint_0 .. SplinePoint_n in the scene.");
            }

            ImGui::Spacing();
            if (DrawCenteredButton(startButtonLabel, 180.0f) && onStartRequested)
            {
                onStartRequested();
            }
            if (DrawCenteredButton("Quit Game", 180.0f) && onStopRequested)
            {
                onStopRequested();
            }

            if (showPerformanceStats)
            {
                ImGui::Separator();
                DrawCenteredText("Press Enter to start quickly");
                ImGui::Text("FPS: %.1f", framesPerSecond);
                ImGui::Text("MS:  %.2f", millisecondsPerFrame);
            }
        }
        ImGui::End();
    }
}
