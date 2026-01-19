//
// Created by Candy on 1/19/2026.
//

#include "DeleteEntityMessage.hpp"
#include "../EngineContext.hpp"

void DeleteEntityMessage::Dispatch(EngineContext &engine)
{
    engine.DeleteEntity(entity);

}
