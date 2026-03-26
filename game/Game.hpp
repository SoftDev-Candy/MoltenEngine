//
// Created by Candy on 2/12/2026.
//

#ifndef B_WENGINE_GAME_HPP
#define B_WENGINE_GAME_HPP

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "../src/Camera.hpp"
#include "../src/Entity.hpp"
#include "../src/Mesh.hpp"
#include "../src/Scene.hpp"
#include "../src/Texture.hpp"

class SplineShooterGame
{
public:
    //Two vibes only: either we freestyle like a menace or ride the spline rails like the assignment wanted//
    enum class ControlMode
    {
        FreeFly,
        SplineFollow
    };

    //Tiny state machine because "game" sounds fancier when it can actually win or lose//
    enum class GameState
    {
        Start,
        Playing,
        Win,
        Lose
    };

    void BindAssets(Mesh* cubeMesh, Texture* defaultTexture);

    void Start(Scene& scene);
    void BeginRound(Scene& scene);
    void Stop(Scene& scene);
    void Update(float deltaTime, Scene& scene, Camera& camera);

    //Input gets piped in from engine land so this class can stay focused on the juicy movement bits//
    void SetInput(float horizontalStrafeInput, float verticalStrafeInput)
    {
        horizontalStrafeInput_ = horizontalStrafeInput;
        verticalStrafeInput_ = verticalStrafeInput;
    }

    void Shoot(Scene& scene);
    void SetMoveForward(float forwardInput) { forwardInput_ = forwardInput; }
    void SetLookDelta(float mouseDeltaX, float mouseDeltaY);
    void SyncViewAngles(float yawDegrees, float pitchDegrees);
    void ToggleControlMode();
    void SetPlayerShipMesh(Mesh* playerShipMesh, const std::string& playerShipMeshKey);
    void SetAsteroidMesh(Mesh* asteroidMesh, const std::string& asteroidMeshKey);
    void SetPreferredPlayerMeshKey(const std::string& preferredPlayerMeshKey) { preferredPlayerMeshKey_ = preferredPlayerMeshKey; }
    void SetPreferredAsteroidMeshKey(const std::string& preferredAsteroidMeshKey) { preferredAsteroidMeshKey_ = preferredAsteroidMeshKey; }

    bool IsRunning() const { return running_; }
    int GetHealth() const { return health_; }
    int GetScore() const { return score_; }
    GameState GetState() const { return state_; }
    ControlMode GetControlMode() const { return controlMode_; }
    const std::string& GetPreferredPlayerMeshKey() const { return preferredPlayerMeshKey_; }
    const std::string& GetPreferredAsteroidMeshKey() const { return preferredAsteroidMeshKey_; }
    const char* GetStateLabel() const;

private:
    // ---- Game State ----
    bool running_ = false;
    GameState state_ = GameState::Start;
    Entity playerEntity_{};
    int health_ = 3;
    int score_ = 0;
    ControlMode controlMode_ = ControlMode::SplineFollow; //Boot into spline mode so the assignment goblin stays happy//

    // ---- Spline Motion ----
    float splineSegmentT_ = 0.0f; //Where we currently are on the spline spaghetti//
    float splineSpeed_ = 0.75f; //Slowed this down a bunch so the ship stops speed-running the whole assignment//
    float horizontalStrafeInput_ = 0.0f;
    float verticalStrafeInput_ = 0.0f;
    float forwardInput_ = 0.0f;
    float laneHalfWidth_ = 2.0f;
    float moveSpeed_ = 6.0f;
    float yaw_ = -90.0f;
    float pitch_ = 0.0f;
    float mouseSensitivity_ = 0.08f;

    // Camera Follow
    float followDistance_ = 4.6f; //Tiny bit farther back so you can breathe and actually see the ship//
    float followHeight_ = 2.2f; //About 10 percent more up because the old camera was a bit too clingy//
    float lookAheadDistance_ = 2.0f;
    float cameraTrackLookAheadT_ = 0.35f; //How far ahead on the spline the camera stares so turns feel clean instead of twitchy//
    float cameraAimHeight_ = 0.35f; //Slight aim lift so the camera looks over the ship and not directly into its skull//
    float cameraFollowResponsiveness_ = 10.0f; //Bigger = snappier, smaller = more floaty space soap//
    bool splineCameraInitialized_ = false;

    // Last Basis From Spline
    glm::vec3 forwardDirection_{0.0f, 0.0f, -1.0f};
    glm::vec3 rightDirection_{1.0f, 0.0f, 0.0f};
    glm::vec3 upDirection_{0.0f, 1.0f, 0.0f};

    // ---- Assets ---- //Borrowed from the engine vault so the bullets dont render as invisible sadness//
    Mesh* cubeMesh_ = nullptr;
    Texture* defaultTexture_ = nullptr;
    Mesh* playerShipMesh_ = nullptr;
    Mesh* asteroidMesh_ = nullptr;
    std::string playerShipMeshKey_ = "Cube"; //Actual thing we ended up using after all the fallback nonsense//
    std::string preferredPlayerMeshKey_ = "D5Class"; //What we WANT if the mesh goblins actually loaded it//
    std::string asteroidMeshKey_ = "Cube";
    std::string preferredAsteroidMeshKey_ = "Asteroid";

    // ---- Bullets & Obstacles ----
    struct Bullet
    {
        Entity entity;
        glm::vec3 velocity;
        float remainingLife = 0.0f;
        float radius = 0.15f;
    };

    struct Obstacle
    {
        Entity entity;
        float radius = 0.45f;
        int hitPoints = 2;
    };

    std::vector<Bullet> activeBullets_;
    std::vector<Obstacle> activeObstacles_;
    int nextBulletIndex_ = 0;
    int nextObstacleIndex_ = 0;

private:
    //Tiny helpers so the main update loop doesnt become one giant cursed lasagna slab//
    SceneObject* FindObjectByName(Scene& scene, const std::string& objectName);
    SceneObject* FindObjectByEntity(Scene& scene, Entity entity);
    bool DestroyObjectByEntity(Scene& scene, Entity entity);
    void ClearRuntimeObjects(Scene& scene);
    SceneObject* EnsurePlayerObject(Scene& scene);
    void PlacePlayerAtTrackStart(Scene& scene, Camera& camera);
    void UpdateSplineCameraRig(
        float deltaTime,
        const std::vector<glm::vec3>& controlPoints,
        float sampledSplineT,
        const glm::vec3& trackCenterPosition,
        const glm::vec3& forwardDirection,
        Camera& camera,
        bool snapImmediately);

    std::vector<glm::vec3> CollectSplineControlPoints(Scene& scene);

    glm::vec3 SampleCatmullRom(const std::vector<glm::vec3>& controlPoints, float splineT) const;
    glm::vec3 TangentCatmullRom(const std::vector<glm::vec3>& controlPoints, float splineT) const;

    static glm::vec3 CatmullRom(
        const glm::vec3& previousPoint,
        const glm::vec3& startPoint,
        const glm::vec3& endPoint,
        const glm::vec3& nextPoint,
        float segmentAlpha);

    void SpawnObstacles(Scene& scene, const std::vector<glm::vec3>& controlPoints);
    void UpdateFreeFly(float deltaTime, SceneObject& playerObject, Camera& camera);
    void UpdateSplineFollow(float deltaTime, SceneObject& playerObject, Scene& scene, Camera& camera);
    void UpdateBullets(float deltaTime, Scene& scene);
    void CheckCollisions(Scene& scene);
};

#endif //B_WENGINE_GAME_HPP
