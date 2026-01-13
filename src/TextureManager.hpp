#ifndef B_WENGINE_TEXTUREMANAGER_HPP
#define B_WENGINE_TEXTUREMANAGER_HPP

#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include "Texture.hpp"

class TextureManager
{
private:
    //Although there are some memory trade-off this auto cleans itself...better that then to do it manually//
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures;

public:
    Texture* Add(const std::string& key, std::unique_ptr<Texture> tex);
    Texture* Get(const std::string& key);
    bool Has(const std::string& key) const; //Just a check incase we don't want errors to be thrown//
    std::vector<std::string> Keys() const;  //for UI listing
    void Clear();                           //IMPORTANT: delete GL stuff BEFORE context dies
};

#endif
