//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_IMPORTTEXTUREMESSAGE_HPP
#define B_WENGINE_IMPORTTEXTUREMESSAGE_HPP

#include "Message.hpp"
#include <string>

struct ImportTextureMessage final : public Message
{
    std::string key;
    std::string path;

    ImportTextureMessage(std::string inKey, std::string inPath)
        : key(std::move(inKey)), path(std::move(inPath)) {}
};

#endif //B_WENGINE_IMPORTTEXTUREMESSAGE_HPP