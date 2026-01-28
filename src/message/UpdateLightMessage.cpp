//
// Created by Candy on 1/28/2026.
//
#include "UpdateLightMessage.hpp"
#include "../EngineContext.hpp"
#include "../Scene.hpp"

void UpdateLightMessage::Dispatch(EngineContext& engine)
{
    auto& lights = engine.GetScene().GetLights();
    if (index < 0 || index >= (int)lights.size()) return;
    lights[index] = data;
}
