//
// Created by Candy on 1/28/2026.
//

#include "AddLightMessage.hpp"
#include "../EngineContext.hpp"
#include "../Scene.hpp"

void AddLightMessage::Dispatch(EngineContext& engine)
{
    engine.GetScene().AddLight();
}
