//
// Created by Candy on 1/28/2026.
//

#ifndef B_WENGINE_UPDATELIGHTMESSAGE_HPP
#define B_WENGINE_UPDATELIGHTMESSAGE_HPP

#include "Message.hpp"
#include "../Scene.hpp" // Light

class UpdateLightMessage : public Message
{
public:
    UpdateLightMessage(int index, const Light& data) : index(index), data(data) {}
    void Dispatch(EngineContext& engine) override;

private:
    int index = -1;
    Light data;
};



#endif //B_WENGINE_UPDATELIGHTMESSAGE_HPP