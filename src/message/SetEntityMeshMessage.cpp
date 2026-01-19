//
// Created by Candy on 1/19/2026.
//

#include "SetEntityMeshMessage.hpp"
#include "../EngineContext.hpp"

void SetEntityMeshMessage::Dispatch(EngineContext& engine)
{
    engine.SetEntityMesh(entity, meshKey);
}
