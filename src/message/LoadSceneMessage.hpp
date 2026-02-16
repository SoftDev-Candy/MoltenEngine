//
// Created by Candy on 2/16/2026.
//

#ifndef B_WENGINE_LOADSCENEMESSAGE_HPP
#define B_WENGINE_LOADSCENEMESSAGE_HPP

#include "Message.hpp"
#include <string>

class LoadSceneMessage : public Message
{
public:
    explicit LoadSceneMessage(std::string path) : path_(std::move(path)) {}
    void Dispatch(EngineContext& engine) override;

private:
    std::string path_;
};


#endif //B_WENGINE_LOADSCENEMESSAGE_HPP