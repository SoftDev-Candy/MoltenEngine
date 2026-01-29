//
// Created by Candy on 1/29/2026.
//

#ifndef B_WENGINE_SETENTITYSPECULARMESSAGE_HPP
#define B_WENGINE_SETENTITYSPECULARMESSAGE_HPP

#include "Message.hpp"
#include "../Entity.hpp"
#include <string>

struct SetEntitySpecularMessage final : public Message
{
    Entity entity;
    std::string specularKey;

    SetEntitySpecularMessage(Entity e, std::string key)
        : entity(e), specularKey(std::move(key)) {}

    void Dispatch(EngineContext& engine) override;
};


#endif //B_WENGINE_SETENTITYSPECULARMESSAGE_HPP