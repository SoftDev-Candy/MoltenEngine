//
// Created by Candy on 1/29/2026.
//

#ifndef B_WENGINE_SETMIPMAPSENABLEDMESSAGE_HPP
#define B_WENGINE_SETMIPMAPSENABLEDMESSAGE_HPP

#include "Message.hpp"

struct SetMipmapsEnabledMessage final : public Message
{
    bool enabled = true;

    explicit SetMipmapsEnabledMessage(bool e)
        : enabled(e) {}

    void Dispatch(EngineContext& engine) override;
};



#endif //B_WENGINE_SETMIPMAPSENABLEDMESSAGE_HPP