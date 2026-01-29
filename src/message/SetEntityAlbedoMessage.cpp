//
// Created by Candy on 1/29/2026.
//

#include "SetEntityAlbedoMessage.hpp"
#include "../EngineContext.hpp"

void SetEntityAlbedoMessage::Dispatch(EngineContext& engine)
{
    engine.SetEntityAlbedo(entity, albedoKey);
}
