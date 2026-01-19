//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_DELETEENTITYMESSAGE_HPP
#define B_WENGINE_DELETEENTITYMESSAGE_HPP

#include "Message.hpp"
#include "../Entity.hpp"


struct DeleteEntityMessage final : public Message
{
    Entity entity;

    explicit DeleteEntityMessage(Entity e)
        : entity(e) {}
    void Dispatch(EngineContext& engine) override; //FIXME: implemented in .cpp

};

#endif //B_WENGINE_DELETEENTITYMESSAGE_HPP