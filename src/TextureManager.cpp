//
// Created by Candy on 1/13/2026.
//
#include "TextureManager.hpp"
#include <algorithm>
#include <utility>

Texture* TextureManager::Add(const std::string& key, std::unique_ptr<Texture> tex)
{
    auto it = textures.find(key);
    if (it != textures.end())
    {
        return it->second.get();
    }

    textures.emplace(key, std::move(tex));
    return textures.at(key).get();
}

Texture* TextureManager::Get(const std::string& key)
{
    auto it = textures.find(key);
    if (it == textures.end())
    {
        return nullptr;
    }
    return it->second.get();
}

bool TextureManager::Has(const std::string& key) const
{
    return textures.find(key) != textures.end();
}

std::vector<std::string> TextureManager::Keys() const
{
    std::vector<std::string> out;
    out.reserve(textures.size());

    for (const auto& kv : textures)
    {
        out.push_back(kv.first);
    }

    std::sort(out.begin(), out.end());
    return out;
}

void TextureManager::Clear()
{
    textures.clear();
}
