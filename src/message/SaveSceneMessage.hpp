//
// Created by Candy on 2/16/2026.
//

#ifndef B_WENGINE_SAVESCENEMESSAGE_HPP
#define B_WENGINE_SAVESCENEMESSAGE_HPP

#include <string>
#include "Message.hpp"


class SaveSceneMessage:public Message
{
public:
    explicit SaveSceneMessage(std::string path) : path_(std::move(path)) {}
    void Dispatch(EngineContext& engine) override;

private:
    std::string path_;
};


#endif //B_WENGINE_SAVESCENEMESSAGE_HPP