//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_FILEDROPPEDMESSAGE_HPP
#define B_WENGINE_FILEDROPPEDMESSAGE_HPP


#include "Message.hpp"
#include <string>
#include <vector>

struct FileDroppedMessage final : public Message
{
    std::vector<std::string> paths;

    FileDroppedMessage() = default;

    // convenience ctor so you can pass paths easily
    explicit FileDroppedMessage(std::vector<std::string> inPaths)
        : paths(std::move(inPaths))
    {
    }
};


#endif //B_WENGINE_FILEDROPPEDMESSAGE_HPP