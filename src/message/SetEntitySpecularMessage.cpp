//
// Created by Candy on 1/29/2026.
//

#include "SetEntitySpecularMessage.hpp"
#include "../EngineContext.hpp"

void SetEntitySpecularMessage::Dispatch(EngineContext& engine)
{
    engine.SetEntitySpecular(entity, specularKey);
}
