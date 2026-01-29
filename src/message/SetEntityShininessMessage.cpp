//
// Created by Candy on 1/29/2026.
//

#include "SetEntityShininessMessage.hpp"
#include "../EngineContext.hpp"

void SetEntityShininessMessage::Dispatch(EngineContext& engine)
{
    engine.SetEntityShininess(entity, shininess);
}
