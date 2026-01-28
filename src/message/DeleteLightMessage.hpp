//
// Created by Candy on 1/28/2026.
//

#ifndef B_WENGINE_DELETELIGHTMESSAGE_HPP
#define B_WENGINE_DELETELIGHTMESSAGE_HPP

#include "Message.hpp"

class DeleteLightMessage : public Message
{
public:
    explicit DeleteLightMessage(int index) : index(index) {}
    void Dispatch(EngineContext& engine) override;

private:
    int index = -1;
};


#endif //B_WENGINE_DELETELIGHTMESSAGE_HPP