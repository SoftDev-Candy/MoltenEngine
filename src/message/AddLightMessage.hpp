//
// Created by Candy on 1/28/2026.
//

#ifndef B_WENGINE_ADDLIGHTMESSAGE_HPP
#define B_WENGINE_ADDLIGHTMESSAGE_HPP
#include "Message.hpp"

class AddLightMessage : public Message
{
public:
    void Dispatch(EngineContext& engine) override;
};



#endif //B_WENGINE_ADDLIGHTMESSAGE_HPP