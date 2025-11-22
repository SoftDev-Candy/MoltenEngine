//
// Created by Candy on 11/20/2025.
//

#ifndef B_WENGINE_SCENE_HPP
#define B_WENGINE_SCENE_HPP
#include <vector>
#include "Renderable.hpp"


class Scene
{
public:
    void Add(Renderable* obj)
    {
        objects.push_back(obj);
    }

    const std::vector<Renderable*>& GetObjects() const
    {
        return objects;
    }

private:
    std::vector<Renderable*>objects;

};


#endif //B_WENGINE_SCENE_HPP