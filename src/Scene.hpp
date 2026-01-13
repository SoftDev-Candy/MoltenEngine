//
// Created by Candy on 11/20/2025.
//

#ifndef B_WENGINE_SCENE_HPP
#define B_WENGINE_SCENE_HPP
#include <string>
#include <vector>
#include "Entity.hpp"
#include "MeshComponent.hpp"
#include"Transform.hpp"

class Texture;//Forward declare it again !

struct SceneObject
{
    Entity entity;
    Transform transform;
    MeshComponent mesh;
    std::string name = "Unnamed";

    //FIXME: this is a bit hacky but its super useful for UI + drag/drop should not be doing this butt🍑 time constraints//
    //Stores the key used inside MeshManager (ex: "Cube", "ImportedOBJ")//
    std::string meshKey = "None";
    //FIXME: keys are for UI + drag/drop (MeshManager/TextureManager lookup)//
    std::string textureKey = "None";

    //Per-entity texture pointer (Renderer will bind this per draw)//
    Texture* texture = nullptr;

};


class Scene
{
public:

Entity CreateObject()
{
        Entity e {nextId++ };
        SceneObject obj;
        obj.entity = e;
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