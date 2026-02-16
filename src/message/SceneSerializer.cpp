//
// Created by Candy on 2/16/2026.
//

#include "SceneSerializer.hpp"
// nlohmann single-header (you placed it in external/nlohmann/json.hpp)
#include <algorithm>
#include <fstream>
#include <iomanip>
#include "../Camera.hpp"
#include "json.hpp"
#include "../Scene.hpp"
#include "glm/vec3.hpp"
#include <filesystem>
#include <iostream>


using json = nlohmann::json;

static json Vec3ToJson(const glm::vec3& v)
{
    return json::array({ v.x, v.y, v.z });
}

static glm::vec3 JsonToVec3(const json& j, const glm::vec3& fallback)
{
    if (!j.is_array() || j.size() != 3) return fallback;
    return glm::vec3(
        j[0].get<float>(),
        j[1].get<float>(),
        j[2].get<float>()
    );
}

namespace SceneSerializer
{
    bool Save(const std::string& path,  Scene& scene, const Camera& cam,
              bool mipmapsEnabled, bool shadowsEnabled)
    {

        std::filesystem::path p(path);
        if (p.has_parent_path())
            std::filesystem::create_directories(p.parent_path());

        json root;
        root["version"] = 1;

        root["settings"] = {
            {"mipmapsEnabled", mipmapsEnabled},
            {"shadowsEnabled", shadowsEnabled}
        };

        root["camera"] = {
            {"position", Vec3ToJson(cam.position)},
            {"rotation", Vec3ToJson(cam.rotation)},
            {"fov", cam.fov}
        };

        // Lights
        root["lights"] = json::array();
        for (const auto& L : scene.GetLights())
        {
            root["lights"].push_back({
                {"position", Vec3ToJson(L.position)},
                {"rotation", Vec3ToJson(L.rotation)},
                {"innerAngle", L.innerAngle},
                {"outerAngle", L.outerAngle},
                {"color", Vec3ToJson(L.color)},
                {"intensity", L.intensity},
                {"ambientStrength", L.ambientStrength}
            });
        }

        // Objects
        root["objects"] = json::array();
        for (const auto& object : scene.GetObjects())
        {
            json jso_nobj;
            jso_nobj["id"]   = object.entity.Id;
            jso_nobj["name"] = object.name;

            jso_nobj["transform"] = {
                {"position", Vec3ToJson(object.transform.position)},
                {"rotation", Vec3ToJson(object.transform.rotation)},
                {"scale",    Vec3ToJson(object.transform.scale)}
            };

            // IMPORTANT: save KEYS, not pointers
            jso_nobj["meshKey"]     = object.meshKey;
            jso_nobj["textureKey"]  = object.textureKey;
            jso_nobj["albedoKey"]   = object.albedoKey;
            jso_nobj["specularKey"] = object.specularKey;
            jso_nobj["shininess"]   = object.shininess;

            root["objects"].push_back(jso_nobj);
        }

        std::ofstream out(path);
        if (!out.is_open())
        {
            std::cout << "[Save] FAILED to open: " << std::filesystem::absolute(p).string() << "\n";
            return false;
        }

        out << std::setw(2) << root;
        return true;
    }

    bool Load(const std::string& path, Scene& scene, Camera& cam,
              bool& mipmapsEnabled, bool& shadowsEnabled)
    {
        std::ifstream in(path);
        if (!in.is_open()) return false;

        json root;
        in >> root;

        // Settings
        if (root.contains("settings"))
        {
            auto& s = root["settings"];
            mipmapsEnabled = s.value("mipmapsEnabled", mipmapsEnabled);
            shadowsEnabled = s.value("shadowsEnabled", shadowsEnabled);
        }

        // Camera
        if (root.contains("camera"))
        {
            auto& c = root["camera"];
            cam.position = JsonToVec3(c.value("position", json{}), cam.position);
            cam.rotation = JsonToVec3(c.value("rotation", json{}), cam.rotation);
            cam.fov      = c.value("fov", cam.fov);
        }

        // Clear current scene (you don’t need new Scene methods for MVP)
        scene.GetObjects().clear();
        scene.GetLights().clear();

        // Lights
        if (root.contains("lights") && root["lights"].is_array())
        {
            for (auto& jl : root["lights"])
            {
                Light L;
                L.position        = JsonToVec3(jl.value("position", json{}), L.position);
                L.rotation        = JsonToVec3(jl.value("rotation", json{}), L.rotation);
                L.innerAngle      = jl.value("innerAngle", L.innerAngle);
                L.outerAngle      = jl.value("outerAngle", L.outerAngle);
                L.color           = JsonToVec3(jl.value("color", json{}), L.color);
                L.intensity       = jl.value("intensity", L.intensity);
                L.ambientStrength = jl.value("ambientStrength", L.ambientStrength);

                scene.GetLights().push_back(L);
            }
        }

        // Objects
        if (root.contains("objects") && root["objects"].is_array())
        {
            for (auto& jo : root["objects"])
            {
                SceneObject o;

                o.entity.Id = jo.value("id", o.entity.Id);
                o.name      = jo.value("name", o.name);

                if (jo.contains("transform"))
                {
                    auto& t = jo["transform"];
                    o.transform.position = JsonToVec3(t.value("position", json{}), o.transform.position);
                    o.transform.rotation = JsonToVec3(t.value("rotation", json{}), o.transform.rotation);
                    o.transform.scale    = JsonToVec3(t.value("scale",    json{}), o.transform.scale);
                }

                o.meshKey     = jo.value("meshKey", o.meshKey);
                o.textureKey  = jo.value("textureKey", o.textureKey);
                o.albedoKey   = jo.value("albedoKey", o.albedoKey);
                o.specularKey = jo.value("specularKey", o.specularKey);
                o.shininess   = jo.value("shininess", o.shininess);

                // pointers will be re-bound by EngineContext after load
                o.mesh.mesh   = nullptr;
                o.texture     = nullptr;
                o.albedo      = nullptr;
                o.specular    = nullptr;

                scene.GetObjects().push_back(o);
            }
        }

        return true;
    }
}
