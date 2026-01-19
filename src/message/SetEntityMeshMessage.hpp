//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_SETENTITYMESHMESSAGE_HPP
#define B_WENGINE_SETENTITYMESHMESSAGE_HPP

#include "Message.hpp"
#include "../Entity.hpp"
#include <string>

struct SetEntityMeshMessage final : public Message
{
    Entity entity;
    std::string meshKey;

    SetEntityMeshMessage(Entity e, std::string key)
        : entity(e), meshKey(std::move(key)) {}

    void Dispatch(EngineContext& engine) override;
};

#endif //B_WENGINE_SETENTITYMESHMESSAGE_HPP
