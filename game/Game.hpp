//
// Created by Candy on 2/12/2026.
//

#ifndef B_WENGINE_GAME_HPP
#define B_WENGINE_GAME_HPP

#include <glm/glm.hpp>
#include <vector>
#include <string>
#include "../src/Scene.hpp"
#include "../src/Camera.hpp"
#include "../src/Mesh.hpp"
#include "../src/Texture.hpp"
#include "../src/Entity.hpp"

class Scene;
class Camera;
class Mesh;
class Texture;
struct SceneObject;
struct Entity;

class SplineShooterGame
{
public:
    void BindAssets(Mesh* cubeMesh, Texture* defaultTex);

    void Start(Scene& scene);
    void Stop(Scene& scene);
    void Update(float dt, Scene& scene, Camera& cam);

    void SetInput(float strafeX, float strafeY) { strafeX_ = strafeX; strafeY_ = strafeY; }
    void Shoot(Scene& scene);
    void DrawHUD();

    bool IsRunning() const { return running_; }

private:
    // ---- game state ----
    bool running_ = false;
    int  health_ = 3;
    int  score_  = 0;

    // ---- spline motion ----
    float seg_t  = 0.0f;     // spline parameter
    float speed_ = 1.5f;     // segments/sec
    float strafeX_ = 0.0f;
    float strafeY_ = 0.0f;
    float laneHalfWidth_ = 2.0f;

    // camera follow
    float followDist_   = 4.0f;
    float followHeight_ = 2.0f;
    float lookAhead_    = 2.0f;

    // last basis from spline
    glm::vec3 fwd_{0,0,-1};
    glm::vec3 right_{1,0,0};
    glm::vec3 up_{0,1,0};

    // ---- assets ----
    Mesh*    cubeMesh_   = nullptr;
    Texture* defaultTex_ = nullptr;

    // ---- bullets & obstacles ----
    struct Bullet
    {
        Entity e;
        glm::vec3 vel;
        float life = 0.0f;
        float radius = 0.15f;
    };

    struct Obstacle
    {
        Entity e;
        float radius = 0.45f;
        int hp = 2;
    };

    std::vector<Bullet> bullets_;
    std::vector<Obstacle> obstacles_;
    int bulletCounter_ = 0;
    int obstacleCounter_ = 0;

private:
    SceneObject* FindByName(Scene& scene, const std::string& name);
    SceneObject* FindByEntity(Scene& scene, Entity e);
    bool DestroyByEntity(Scene& scene, Entity e);

    std::vector<glm::vec3> CollectSplinePoints(Scene& scene);

    glm::vec3 SampleCatmullRom(const std::vector<glm::vec3>& pts, float t) const;
    glm::vec3 TangentCatmullRom(const std::vector<glm::vec3>& pts, float t) const;

    static glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                const glm::vec3& p2, const glm::vec3& p3, float u);

    void SpawnObstacles(Scene& scene, const std::vector<glm::vec3>& pts);
    void UpdateBullets(float dt, Scene& scene);
    void CheckCollisions(Scene& scene);
};
#endif //B_WENGINE_GAME_HPP