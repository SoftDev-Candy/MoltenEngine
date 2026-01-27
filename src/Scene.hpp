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

struct Light
{
    glm::vec3 position{ 2.0f, 3.0f, 2.0f };
    glm::vec3 color{ 1.0f, 1.0f, 1.0f };
    float intensity = 2.0f;        // makes it brighter
    float ambientStrength = 0.15f; // per-light ambient
};


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

    // --- Material (Phase 2A) ---
    std::string albedoKey = "None";
    std::string specularKey = "None";

    Texture* albedo = nullptr;     // diffuse/albedo map
    Texture* specular = nullptr;   // specular mask map
    float shininess = 32.0f;       // phong exponent



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

    //Delete by index (UI uses selectedIndex a lot so this is easiest)
    bool DestroyObjectAt(size_t index)
{
    if (index >= objects.size())
    {
        return false;
    }

    objects.erase(objects.begin() + index);
    return true;
}

    //Optional: Delete by Entity (handy later)
    bool DestroyObject(Entity e)
{
    for (size_t i = 0; i < objects.size(); i++)
    {
        //If this line errors, it means your Entity doesn't have ".id"
        //Then change it to whatever your ID field is.
        if (objects[i].entity.Id == e.Id)
        {
            objects.erase(objects.begin() + i);
            return true;
        }
    }
    return false;
}

//Improved delete
    bool DeleteEntity(Entity e, int& selectedIndex)
{
    for (int i = 0; i < (int)objects.size(); i++)
    {
        if (objects[i].entity.Id == e.Id)
        {
            objects.erase(objects.begin() + i);

            //Fix selection so it doesn't point into the void
            if (objects.empty())
                selectedIndex = -1;
            else if (selectedIndex >= (int)objects.size())
                selectedIndex = (int)objects.size() - 1;
            else if (selectedIndex == i)
                selectedIndex = -1;

            return true;
        }
    }
    return false;
}


private:

    std::vector<SceneObject>objects;
    EntityID nextId = 1;
    std::vector<Light> lights;


};

#endif //B_WENGINE_SCENE_HPP