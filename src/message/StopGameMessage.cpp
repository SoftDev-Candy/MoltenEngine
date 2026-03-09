//
// Created by Candy on 3/9/2026.
//

#include "StopGameMessage.hpp"
#include "../EngineContext.hpp"

void StopGameMessage::Dispatch(EngineContext &engine)
{
    engine.StopGame();
}
