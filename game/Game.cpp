#include "Game.hpp"

#include "../src/Scene.hpp"
#include "../src/Camera.hpp"
#include "../src/Mesh.hpp"
#include "../src/Texture.hpp"
#include "../src/Entity.hpp"

#include <imgui.h>
#include <iostream>
#include <algorithm>
#include <cctype>
#include <cmath>

void SplineShooterGame::BindAssets(Mesh* cubeMesh, Texture* defaultTex)
{
    //Snag the shiny toys from engine land so gameplay doesnt throw invisible cubes at the player//
    cubeMesh_ = cubeMesh;
    defaultTex_ = defaultTex;
}

SceneObject* SplineShooterGame::FindByName(Scene& scene, const std::string& name)
{
    for (auto& o : scene.GetObjects())
        if (o.name == name) return &o;
    return nullptr;
}

SceneObject* SplineShooterGame::FindByEntity(Scene& scene, Entity e)
{
    for (auto& o : scene.GetObjects())
        if (o.entity.Id == e.Id) return &o;
    return nullptr;
}

bool SplineShooterGame::DestroyByEntity(Scene& scene, Entity e)
{
    auto& objs = scene.GetObjects();
    for (size_t i = 0; i < objs.size(); ++i)
    {
        if (objs[i].entity.Id == e.Id)
        {
            scene.DestroyObjectAt(i);
            return true;
        }
    }
    return false;
}

// Name format: SplinePoint_0, SplinePoint_1... because naming things is half the boss fight//
static bool StartsWith(const std::string& s, const char* prefix)
{
    return s.rfind(prefix, 0) == 0;
}

static int ParseSuffixInt(const std::string& s)
{
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

    std::sort(tmp.begin(), tmp.end(), [](auto& a, auto& b){ return a.first < b.first; });

    std::vector<glm::vec3> pts;
    pts.reserve(tmp.size());
    for (auto& it : tmp) pts.push_back(it.second);
    return pts;
}

glm::vec3 SplineShooterGame::CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                       const glm::vec3& p2, const glm::vec3& p3, float u)
{
    float u2 = u * u;
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

    int i0 = std::clamp(seg, 0, (int)pts.size() - 4);
    return CatmullRom(pts[i0], pts[i0+1], pts[i0+2], pts[i0+3], u);
}

glm::vec3 SplineShooterGame::TangentCatmullRom(const std::vector<glm::vec3>& pts, float t) const
{
    const float eps = 0.01f;
    glm::vec3 a = SampleCatmullRom(pts, std::max(0.0f, t - eps));
    glm::vec3 b = SampleCatmullRom(pts, std::min(t + eps, (float)(pts.size() - 3)));
    return b - a;
}

void SplineShooterGame::SpawnObstacles(Scene& scene, const std::vector<glm::vec3>& pts)
{
    obstacles_.clear();
    obstacleCounter_ = 0;

    if ((int)pts.size() < 4) return;

    float maxT = (float)(pts.size() - 3);
    if (maxT <= 0.0f) return;

    // place obstacles along the track (simple pattern)
    for (float t = 0.5f; t < maxT; t += 0.7f)
    {
        scene.CreateObject();
        auto& o = scene.GetObjects().back();

        o.name = "Obstacle_" + std::to_string(obstacleCounter_++);
        o.mesh.mesh = cubeMesh_;
        o.meshKey = "Cube";

        o.texture = defaultTex_;
        o.textureKey = "Default";
        o.albedo = defaultTex_;
        o.albedoKey = "Default";
        o.specular = defaultTex_;
        o.specularKey = "Default";
        o.shininess = 16.0f;

        glm::vec3 pos = SampleCatmullRom(pts, t);
        glm::vec3 fwd = glm::normalize(TangentCatmullRom(pts, t));
        glm::vec3 right = glm::normalize(glm::cross(fwd, glm::vec3(0,1,0)));

        float lane = ((obstacleCounter_ % 3) - 1) * 1.0f; // -1,0,1 lanes
        pos += right * lane;

        o.transform.position = pos;
        o.transform.scale = glm::vec3(0.6f);

        obstacles_.push_back({ o.entity, 0.55f, 2 });
    }
}

void SplineShooterGame::Start(Scene& scene)
{
    running_ = true;
    health_ = 3;
    score_ = 0;
    seg_t = 0.0f;

    // Ensure player exists
    if (!FindByName(scene, "Player"))
    {
        scene.CreateObject();
        auto& o = scene.GetObjects().back();
        o.name = "Player";
        o.mesh.mesh = cubeMesh_;
        o.meshKey = "Cube";

        o.texture = defaultTex_;
        o.textureKey = "Default";
        o.albedo = defaultTex_;
        o.albedoKey = "Default";
        o.specular = defaultTex_;
        o.specularKey = "Default";
        o.shininess = 64.0f;

        o.transform.scale = glm::vec3(0.35f);
    }

    // build obstacles from current spline
    auto pts = CollectSplinePoints(scene);
    SpawnObstacles(scene, pts);

    bullets_.clear();
    bulletCounter_ = 0;
}

void SplineShooterGame::Stop(Scene& scene)
{
    running_ = false;

    // clean bullets + obstacles (keep spline points + player)
    for (auto& b : bullets_)  DestroyByEntity(scene, b.e);
    for (auto& o : obstacles_) DestroyByEntity(scene, o.e);

    bullets_.clear();
    obstacles_.clear();
}

void SplineShooterGame::Shoot(Scene& scene)
{
    if (!running_) return;
    SceneObject* player = FindByName(scene, "Player");
    if (!player) return;

    scene.CreateObject();
    auto& o = scene.GetObjects().back();

    o.name = "Bullet_" + std::to_string(bulletCounter_++);
    o.mesh.mesh = cubeMesh_;
    o.meshKey = "Cube";

    o.texture = defaultTex_;
    o.textureKey = "Default";
    o.albedo = defaultTex_;
    o.albedoKey = "Default";
    o.specular = defaultTex_;
    o.specularKey = "Default";
    o.shininess = 8.0f;

    o.transform.scale = glm::vec3(0.12f);
    o.transform.position = player->transform.position + fwd_ * 0.7f;

    Bullet b;
    b.e = o.entity;
    b.vel = fwd_ * 12.0f;
    b.life = 2.0f;
    b.radius = 0.15f;

    bullets_.push_back(b);
}

void SplineShooterGame::UpdateBullets(float dt, Scene& scene)
{
    for (int i = (int)bullets_.size() - 1; i >= 0; --i)
    {
        bullets_[i].life -= dt;

        SceneObject* o = FindByEntity(scene, bullets_[i].e);
        if (!o || bullets_[i].life <= 0.0f)
        {
            if (o) DestroyByEntity(scene, bullets_[i].e);
            bullets_.erase(bullets_.begin() + i);
            continue;
        }

        o->transform.position += bullets_[i].vel * dt;
    }
}

static float Dist(const glm::vec3& a, const glm::vec3& b)
{
    glm::vec3 d = a - b;
    return std::sqrt(d.x*d.x + d.y*d.y + d.z*d.z);
}

void SplineShooterGame::CheckCollisions(Scene& scene)
{
    SceneObject* player = FindByName(scene, "Player");
    if (!player) return;

    // Player vs Obstacles
    for (int oi = (int)obstacles_.size() - 1; oi >= 0; --oi)
    {
        SceneObject* obsObj = FindByEntity(scene, obstacles_[oi].e);
        if (!obsObj)
        {
            obstacles_.erase(obstacles_.begin() + oi);
            continue;
        }

        float d = Dist(player->transform.position, obsObj->transform.position);
        if (d < (0.35f + obstacles_[oi].radius))
        {
            health_ -= 1;
            DestroyByEntity(scene, obstacles_[oi].e);
            obstacles_.erase(obstacles_.begin() + oi);

            if (health_ <= 0)
            {
                running_ = false;
                return;
            }
        }
    }

    // Bullets vs Obstacles
    for (int bi = (int)bullets_.size() - 1; bi >= 0; --bi)
    {
        SceneObject* bObj = FindByEntity(scene, bullets_[bi].e);
        if (!bObj) continue;

        bool bulletKilled = false;

        for (int oi = (int)obstacles_.size() - 1; oi >= 0; --oi)
        {
            SceneObject* obsObj = FindByEntity(scene, obstacles_[oi].e);
            if (!obsObj) continue;

            float d = Dist(bObj->transform.position, obsObj->transform.position);
            if (d < (bullets_[bi].radius + obstacles_[oi].radius))
            {
                obstacles_[oi].hp -= 1;

                DestroyByEntity(scene, bullets_[bi].e);
                bullets_.erase(bullets_.begin() + bi);
                bulletKilled = true;

                if (obstacles_[oi].hp <= 0)
                {
                    DestroyByEntity(scene, obstacles_[oi].e);
                    obstacles_.erase(obstacles_.begin() + oi);
                    score_ += 10;
                }
                break;
            }
        }

        if (bulletKilled) continue;
    }
}

void SplineShooterGame::Update(float dt, Scene& scene, Camera& cam)
{
    if (!running_) return;

    auto pts = CollectSplinePoints(scene);
    if ((int)pts.size() < 4) return;

    SceneObject* player = FindByName(scene, "Player");
    if (!player) return;

    // move along spline
    seg_t += speed_ * dt;

    float maxT = (float)(pts.size() - 3);
    if (maxT <= 0.0f) return;
    while (seg_t > maxT) seg_t -= maxT;

    glm::vec3 pos = SampleCatmullRom(pts, seg_t);
    glm::vec3 fwd = glm::normalize(TangentCatmullRom(pts, seg_t));

    glm::vec3 worldUp(0,1,0);
    glm::vec3 right = glm::normalize(glm::cross(fwd, worldUp));
    glm::vec3 up    = glm::normalize(glm::cross(right, fwd));

    // store basis for shooting
    fwd_ = fwd; right_ = right; up_ = up;

    // strafe (clamped)
    float sx = std::clamp(strafeX_, -1.0f, 1.0f);
    float sy = std::clamp(strafeY_, -1.0f, 1.0f);

    pos += right * sx * laneHalfWidth_;
    pos += up    * sy * (laneHalfWidth_ * 0.6f);

    player->transform.position = pos;

    // follow camera
    glm::vec3 camPos = pos - fwd * followDist_ + worldUp * followHeight_;
    glm::vec3 target = pos + fwd * lookAhead_;

    cam.position = camPos;

    glm::vec3 dir = glm::normalize(target - camPos);
    cam.rotation.y = glm::degrees(std::atan2(dir.z, dir.x));
    cam.rotation.x = glm::degrees(std::asin(dir.y));

    // bullets + collisions
    UpdateBullets(dt, scene);
    CheckCollisions(scene);
}

void SplineShooterGame::DrawHUD()
{
    //Simple overlay living in the basement like a backup goblin if we want game-only HUD later//
    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoDecoration |
        ImGuiWindowFlags_NoDocking |
        ImGuiWindowFlags_AlwaysAutoResize |
        ImGuiWindowFlags_NoSavedSettings |
        ImGuiWindowFlags_NoFocusOnAppearing |
        ImGuiWindowFlags_NoNav;

    ImGui::SetNextWindowPos(ImVec2(12, 52), ImGuiCond_Always);
    ImGui::SetNextWindowBgAlpha(0.35f);

    ImGui::Begin("HUD##SplineShooter", nullptr, flags);
    ImGui::Text("Health: %d", health_);
    ImGui::Text("Score:  %d", score_);
    ImGui::Text("Shoot: SPACE | Strafe: A/D | Up/Down: Q/E");
    if (!running_)
        ImGui::Text("Status: STOPPED (Play again)");
    ImGui::End();
}
