//
// Created by Candy on 3/9/2026.
//

#include "StartGameMessage.hpp"
#include "../EngineContext.hpp"
void StartGameMessage::Dispatch(EngineContext &engine)
{
    engine.StartGame();
}

