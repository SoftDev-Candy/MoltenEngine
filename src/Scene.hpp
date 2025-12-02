//
// Created by Candy on 11/20/2025.
//

#ifndef B_WENGINE_SCENE_HPP
#define B_WENGINE_SCENE_HPP
#include <vector>
#include "Entity.hpp"
#include"Transform.hpp"
#include "Renderable.hpp"

struct SceneObject
{
    Entity entity;
    Transform transform;
    Renderable* renderable = nullptr;
};


class Scene
{
public:
Entity CreateObject(Renderable* r)
{
        Entity e {nextId++ };
        SceneObject obj;
        obj.entity = e;
        obj.renderable = r;
        objects.push_back(obj);
        return e;
}

    std::vector<SceneObject>& GetObjects()
    {
        return objects;
    }

private:
    std::vector<SceneObject>objects;
    EntityID nextId = 1;
};


#endif //B_WENGINE_SCENE_HPP