//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_IMPORTMESHMESSAGE_HPP
#define B_WENGINE_IMPORTMESHMESSAGE_HPP

#include "Message.hpp"
#include <string>

struct ImportMeshMessage final : public Message
{
    std::string key;
    std::string path;

    ImportMeshMessage(std::string k, std::string p)
        : key(std::move(k)), path(std::move(p)) {}

    void Dispatch(EngineContext& engine) override;
};

#endif //B_WENGINE_IMPORTMESHMESSAGE_HPP
