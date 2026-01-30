//
// Created by Candy on 1/29/2026.
//

#include "SetMipmapsEnabledMessage.hpp"
#include "../EngineContext.hpp"

void SetMipmapsEnabledMessage::Dispatch(EngineContext& engine)
{
    engine.SetMipmapsEnabled(enabled);
}
