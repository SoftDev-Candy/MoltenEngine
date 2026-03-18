#ifndef B_WENGINE_GAMEUI_HPP
#define B_WENGINE_GAMEUI_HPP

#include <functional>

class SplineShooterGame;

namespace GameUI
{
    //This stays in game-land on purpose so the play HUD doesnt get lost inside editor spaghetti//
    void DrawPlayHUD(
        const SplineShooterGame& game,
        float framesPerSecond,
        float millisecondsPerFrame,
        bool showPerformanceStats,
        const std::function<void()>& onStartRequested,
        const std::function<void()>& onStopRequested);
}

#endif //B_WENGINE_GAMEUI_HPP
