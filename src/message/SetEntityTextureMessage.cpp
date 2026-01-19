//
// Created by Candy on 1/19/2026.
//

#include "SetEntityTextureMessage.hpp"
#include "../EngineContext.hpp"

void SetEntityTextureMessage::Dispatch(EngineContext& engine)
{
    engine.SetEntityTexture(entity, textureKey);
}
