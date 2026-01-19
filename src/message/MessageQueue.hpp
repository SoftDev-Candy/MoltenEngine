//
// Created by Candy on 1/19/2026.
//

#ifndef B_WENGINE_MESSAGEQUEUE_HPP
#define B_WENGINE_MESSAGEQUEUE_HPP

#include <vector>
#include <memory>
#include <mutex>
#include "Message.hpp"

class MessageQueue
{
public:
    // Push a message into the queue (ownership transferred)
    void Push(std::unique_ptr<Message> msg)
    {
        if (!msg)
            return;

        std::lock_guard<std::mutex> lock(mutex);
        messages.emplace_back(std::move(msg));
    }

    // Drain all messages at once (so update() can process them)
    std::vector<std::unique_ptr<Message>> PopAll()
    {
        std::lock_guard<std::mutex> lock(mutex);

        std::vector<std::unique_ptr<Message>> out;
        out.swap(messages); // fast + leaves messages empty

        return out;
    }

    // Optional helpers (good for debug + memory checks)
    bool Empty() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return messages.empty();
    }

    size_t Size() const
    {
        std::lock_guard<std::mutex> lock(mutex);
        return messages.size();
    }

    void Clear()
    {
        std::lock_guard<std::mutex> lock(mutex);
        messages.clear();
    }

private:
    mutable std::mutex mutex;
    std::vector<std::unique_ptr<Message>> messages;
};

#endif //B_WENGINE_MESSAGEQUEUE_HPP