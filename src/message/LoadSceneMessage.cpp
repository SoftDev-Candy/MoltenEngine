//
// Created by Candy on 2/16/2026.
//

#include "LoadSceneMessage.hpp"
#include "../EngineContext.hpp"

void LoadSceneMessage::Dispatch(EngineContext& engine)
{
    engine.LoadScene(path_);
}
