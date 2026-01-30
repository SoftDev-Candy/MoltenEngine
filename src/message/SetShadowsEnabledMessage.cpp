//
// Created by Candy on 1/30/2026.
//

#include "SetShadowsEnabledMessage.hpp"

#include "SetShadowsEnabledMessage.hpp"
#include "../EngineContext.hpp"

void SetShadowsEnabledMessage::Dispatch(EngineContext& engine)
{
    engine.SetShadowsEnabled(enabled);
}
