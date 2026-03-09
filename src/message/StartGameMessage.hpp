//
// Created by Candy on 3/9/2026.
//

#ifndef B_WENGINE_STARTGAMEMESSAGE_HPP
#define B_WENGINE_STARTGAMEMESSAGE_HPP

#include"Message.hpp"

struct StartGameMessage : public Message
{
    void Dispatch(EngineContext &engine) override;
};

#endif //B_WENGINE_STARTGAMEMESSAGE_HPP