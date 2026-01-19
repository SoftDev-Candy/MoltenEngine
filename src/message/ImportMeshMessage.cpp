//
// Created by Candy on 1/19/2026.
//

#include "ImportMeshMessage.hpp"
#include "../EngineContext.hpp"

void ImportMeshMessage::Dispatch(EngineContext& engine)
{
    engine.ImportObjAsMesh(key, path);
}
