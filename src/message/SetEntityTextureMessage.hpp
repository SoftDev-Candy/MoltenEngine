//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_SETENTITYTEXTUREMESSAGE_HPP
#define B_WENGINE_SETENTITYTEXTUREMESSAGE_HPP

#include "Message.hpp"
#include "../Entity.hpp"
#include <string>

struct SetEntityTextureMessage final : public Message
{
    Entity entity;
    std::string textureKey;

    SetEntityTextureMessage(Entity e, std::string key)
        : entity(e), textureKey(std::move(key)) {}

    void Dispatch(EngineContext& engine) override;
};

#endif //B_WENGINE_SETENTITYTEXTUREMESSAGE_HPP
