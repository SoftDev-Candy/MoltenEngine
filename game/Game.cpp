//
// Created by Candy on 2/12/2026.
//

#include "Game.hpp"

#include "../src/Scene.hpp"
#include "../src/Camera.hpp"
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>

void SplineShooterGame::Start(Scene& scene)
{
    running_ = true;
    seg_t = 0.0f;

    //Checking here to see if the player does exist
    if (!FindByName(scene, "Player"))
    {
        scene.CreateObject();
        auto& o = scene.GetObjects().back();
        o.name = "Player";
        o.meshKey = "Cube";
        o.transform.scale = glm::vec3(0.35f);
    }
}

//Is Da Game Running Son
void SplineShooterGame::Stop()
{
    running_ = false;
}

void SplineShooterGame::Update(float dt, Scene& scene, Camera& cam)
{
    if (!running_) return;
    speed_ = 2.5f; // just for testing
    auto pts = CollectSplinePoints(scene);


    if ((int)pts.size() < 4) return; // need at least 4 for Catmull-Rom

    SceneObject* player = FindByName(scene, "Player");
    if (!player) return;

    // advance along spline
    seg_t += speed_ * dt;

    // clamp to end for MVP (later: win state)
    float maxT = (float)(pts.size() - 3);
    if (maxT <= 0.0f) return;

    // loop forever
    while (seg_t > maxT) seg_t -= maxT;

    glm::vec3 pos = SampleCatmullRom(pts, seg_t);
    glm::vec3 fwd = glm::normalize(TangentCatmullRom(pts, seg_t));

    glm::vec3 worldUp(0,1,0);
    glm::vec3 right = glm::normalize(glm::cross(fwd, worldUp));
    glm::vec3 up    = glm::normalize(glm::cross(right, fwd));

    // strafe offsets (editor feel)
    pos += right * strafeX_ * 2.0f;
    pos += up    * strafeY_ * 2.0f;

    player->transform.position = pos;

    // Camera follow
    glm::vec3 camPos = pos - fwd * followDist_ + worldUp * followHeight_;
    glm::vec3 target = pos + fwd * lookAhead_;

    cam.position = camPos;

    glm::vec3 dir = glm::normalize(target - camPos);
    cam.rotation.y = glm::degrees(std::atan2(dir.z, dir.x)); // yaw
    cam.rotation.x = glm::degrees(std::asin(dir.y));         // pitch
}

SceneObject* SplineShooterGame::FindByName(Scene& scene, const std::string& name)
{
    for (auto& o : scene.GetObjects())
        if (o.name == name) return &o;
    return nullptr;
}

// name format: SplinePoint_0, SplinePoint_1...find da spline please
static bool StartsWith(const std::string& s, const char* prefix)
{
    return s.rfind(prefix, 0) == 0;
}

static int ParseSuffixInt(const std::string& s)
{
    // extract trailing digits
    int i = (int)s.size() - 1;
    while (i >= 0 && std::isdigit((unsigned char)s[i])) i--;
    if (i == (int)s.size() - 1) return -1;
    return std::stoi(s.substr(i + 1));
}

std::vector<glm::vec3> SplineShooterGame::CollectSplinePoints(Scene& scene)
{
    std::vector<std::pair<int, glm::vec3>> tmp;

    for (auto& o : scene.GetObjects())
    {
        if (!StartsWith(o.name, "SplinePoint_")) continue;
        int idx = ParseSuffixInt(o.name);
        if (idx < 0) continue;
        tmp.push_back({ idx, o.transform.position });
    }

    //I dont like this I know what is wrong with it and no me neither
    std::sort(tmp.begin(), tmp.end(), [](auto& a, auto& b)
    {
        return a.first < b.first;
    });

    std::vector<glm::vec3> pts;
    pts.reserve(tmp.size());
    for (auto& it : tmp)
    {
        pts.push_back(it.second);
    }
    return pts;
}

glm::vec3 SplineShooterGame::CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                       const glm::vec3& p2, const glm::vec3& p3, float u)
{
    float u2 = u * u; //  u u u u u u u uu u  you i dont like it
    float u3 = u2 * u;

    return 0.5f * (
        (2.0f * p1) +
        (-p0 + p2) * u +
        (2.0f*p0 - 5.0f*p1 + 4.0f*p2 - p3) * u2 +
        (-p0 + 3.0f*p1 - 3.0f*p2 + p3) * u3
    );
}

glm::vec3 SplineShooterGame::SampleCatmullRom(const std::vector<glm::vec3>& pts, float t) const
{
    int seg = (int)std::floor(t);
    float u = t - (float)seg;

    // for Catmull-Rom: use p0..p3 = pts[seg..seg+3] // SPLINES ! are pretty mathematical ngl
    int i0 = std::clamp(seg, 0, (int)pts.size() - 4);
    return CatmullRom(pts[i0], pts[i0+1], pts[i0+2], pts[i0+3], u);
}

glm::vec3 SplineShooterGame::TangentCatmullRom(const std::vector<glm::vec3>& pts, float t) const
{
    // simple numeric derivative (fast MVP)
    const float eps = 0.01f;
    glm::vec3 a = SampleCatmullRom(pts, std::max(0.0f, t - eps));
    glm::vec3 b = SampleCatmullRom(pts, std::min(t + eps, (float)(pts.size() - 3)));
    return b - a;
}