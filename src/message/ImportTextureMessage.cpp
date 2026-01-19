//
// Created by Candy on 1/19/2026.
//

#include "ImportTextureMessage.hpp"
//
// Created by Candy on 1/19/2026.
//

#include "ImportTextureMessage.hpp"
#include "../EngineContext.hpp"

void ImportTextureMessage::Dispatch(EngineContext& engine)
{
    engine.ImportTexture(key, path);
}
