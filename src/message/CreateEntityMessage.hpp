//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_CREATEENTITYMESSAGE_HPP
#define B_WENGINE_CREATEENTITYMESSAGE_HPP

#include "Message.hpp"
#include <string>

struct CreateEntityMessage final : public Message
{
    std::string name;
    std::string meshKey;

    CreateEntityMessage(std::string n, std::string mk)
        : name(std::move(n)), meshKey(std::move(mk)) {}

    void Dispatch(EngineContext& engine) override;
};

#endif //B_WENGINE_CREATEENTITYMESSAGE_HPP