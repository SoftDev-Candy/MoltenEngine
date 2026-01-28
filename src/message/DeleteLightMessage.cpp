//
// Created by Candy on 1/28/2026.
//

#include "DeleteLightMessage.hpp"
#include "../EngineContext.hpp"
#include "../Scene.hpp"

void DeleteLightMessage::Dispatch(EngineContext& engine)
{
    engine.GetScene().RemoveLight(index);
}
