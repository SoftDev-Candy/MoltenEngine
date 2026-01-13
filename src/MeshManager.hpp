//
// Created by Candy on 1/3/2026.
//

#ifndef B_WENGINE_MESHMANAGER_HPP
#define B_WENGINE_MESHMANAGER_HPP
#include <memory>
#include <unordered_map>
#include <string>
#include <vector>
#include "Mesh.hpp"

class MeshManager
{
private:
    //Although there are some memory trade-off this auto cleans itself...better that then to do it manually//
std::unordered_map<std::string , std::unique_ptr<Mesh>> meshes;

    //Add API's in here for add and get //
public:

    Mesh* Add(const std::string key , std::unique_ptr<Mesh> mesh);
    Mesh* Get(const std::string& key);
    bool Has(const std::string& key) const; //Just a check incase we don't want errors to be thrown//
    std::vector<std::string> Keys() const; //for UI listing
    void Clear(); //IMPORTANT: delete GL stuff BEFORE context dies




};


#endif //B_WENGINE_MESHMANAGER_HPP