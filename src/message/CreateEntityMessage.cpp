//
// Created by Candy on 1/19/2026.
//

#include "CreateEntityMessage.hpp"
#include "../EngineContext.hpp"


void CreateEntityMessage::Dispatch(EngineContext &engine)
{
    engine.CreateEntityWithMesh(name, meshKey);
}
