#include "GameUI.hpp"

#include "Game.hpp"
#include <imgui.h>

namespace GameUI
{
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

        //Tiny cockpit HUD lives here now so gameplay code can stop cosplaying as a UI file//
        ImGui::SetNextWindowViewport(mainViewport->ID);
        ImGui::SetNextWindowPos(
            ImVec2(mainViewport->WorkPos.x + 12.0f, mainViewport->WorkPos.y + 12.0f),
            ImGuiCond_Always);
        ImGui::SetNextWindowBgAlpha(0.35f);

        if (ImGui::Begin("PlayHUD##SplineShooter", nullptr, hudWindowFlags))
        {
            ImGui::TextUnformatted("PLAY MODE");
            ImGui::Separator();
            ImGui::Text("State:  %s", game.GetStateLabel());
            ImGui::Text("Health: %d", game.GetHealth());
            ImGui::Text("Score:  %d", game.GetScore());

            if (game.GetState() == SplineShooterGame::GameState::Playing)
            {
                const char* controlModeLabel =
                    (game.GetControlMode() == SplineShooterGame::ControlMode::FreeFly)
                        ? "FreeFly"
                        : "SplineFollow";

                ImGui::Text("Mode:   %s", controlModeLabel);
                ImGui::TextUnformatted("Move: A/D strafe | Q/E rise | SPACE shoot");
                ImGui::TextUnformatted("Press F if you want to flip movement modes. ESC bails out.");
            }
            else if (game.GetState() == SplineShooterGame::GameState::Start)
            {
                ImGui::TextUnformatted("Press Enter to start the spline run.");
                ImGui::TextUnformatted("Need at least 4 objects named SplinePoint_0..N.");

                if (ImGui::Button("Start Run") && onStartRequested)
                {
                    onStartRequested();
                }
            }
            else if (game.GetState() == SplineShooterGame::GameState::Win)
            {
                ImGui::TextUnformatted("You cleared the track. Nice.");

                if (ImGui::Button("Restart") && onStartRequested)
                {
                    onStartRequested();
                }
            }
            else if (game.GetState() == SplineShooterGame::GameState::Lose)
            {
                ImGui::TextUnformatted("Ship exploded. Boom BOOM.");
                ImGui::TextUnformatted("Press Enter or hit Restart to play again.");

                if (ImGui::Button("Restart") && onStartRequested)
                {
                    onStartRequested();
                }
            }

            if (showPerformanceStats)
            {
                //Performance numbers because we all like pretending we are benchmarking NASA software//
                ImGui::Separator();
                ImGui::Text("FPS: %.1f", framesPerSecond);
                ImGui::Text("MS:  %.2f", millisecondsPerFrame);
            }

            if (game.GetState() != SplineShooterGame::GameState::Playing)
            {
                ImGui::Separator();
            }

            if (ImGui::Button("Stop") && onStopRequested)
            {
                onStopRequested();
            }
        }
        ImGui::End();
    }
}
