//
// Created by Candy on 3/9/2026.
//

#ifndef B_WENGINE_STOPGAMEMESSAGE_HPP
#define B_WENGINE_STOPGAMEMESSAGE_HPP

#include "Message.hpp"

struct StopGameMessage : public Message
{
    void Dispatch(EngineContext &engine) override;

};


#endif //B_WENGINE_STOPGAMEMESSAGE_HPP