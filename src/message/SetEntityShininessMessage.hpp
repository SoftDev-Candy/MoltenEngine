//
// Created by Candy on 1/29/2026.
//

#ifndef B_WENGINE_SETENTITYSHININESSMESSAGE_HPP
#define B_WENGINE_SETENTITYSHININESSMESSAGE_HPP

#include "Message.hpp"
#include "../Entity.hpp"

struct SetEntityShininessMessage final : public Message
{
    Entity entity;
    float shininess = 32.0f;

    SetEntityShininessMessage(Entity e, float s)
        : entity(e), shininess(s) {}

    void Dispatch(EngineContext& engine) override;
};

#endif //B_WENGINE_SETENTITYSHININESSMESSAGE_HPP