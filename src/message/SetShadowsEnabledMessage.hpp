//
// Created by Candy on 1/30/2026.
//

#ifndef B_WENGINE_SETSHADOWSENABLEDMESSAGE_HPP
#define B_WENGINE_SETSHADOWSENABLEDMESSAGE_HPP

#include "Message.hpp"

struct SetShadowsEnabledMessage final : public Message
{
    bool enabled = true;
    explicit SetShadowsEnabledMessage(bool e) : enabled(e) {}

    void Dispatch(EngineContext& engine) override;
};


#endif //B_WENGINE_SETSHADOWSENABLEDMESSAGE_HPP