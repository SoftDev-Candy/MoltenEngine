//
// Created by Candy on 2/16/2026.
//

#include "SaveSceneMessage.hpp"
#include "../EngineContext.hpp"


void SaveSceneMessage::Dispatch(EngineContext &engine)
{
    engine.SaveScene(path_);
}
