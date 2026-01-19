//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_MESSAGE_HPP
#define B_WENGINE_MESSAGE_HPP

class EngineContext; //forward declare so we don’t include EngineContext.hpp here

struct Message
{
    virtual ~Message() = default;

    //Every message must be able to execute inside the engine
    virtual void Dispatch(EngineContext& engine) = 0;
};

#endif //B_WENGINE_MESSAGE_HPP
