#include "Game.hpp"

#include "../src/Camera.hpp"
#include "../src/Entity.hpp"
#include "../src/Mesh.hpp"
#include "../src/Scene.hpp"
#include "../src/Texture.hpp"

#include <algorithm>
#include <cctype>
#include <cmath>

static bool StartsWithPrefix(const std::string& text, const char* prefix)
{
    return text.rfind(prefix, 0) == 0;
}

static float FractionalPart(float numberValue)
{
    return numberValue - std::floor(numberValue);
}

static float StableNoise01(int indexValue, int saltValue)
{
    //Fake random but deterministic, so the asteroid soup stays consistent instead of shuffling every launch like a goblin//
    float weirdSeed = std::sin((float)(indexValue * 127 + saltValue * 53)) * 43758.5453f;
    return FractionalPart(weirdSeed);
}

static int PickLaneIndex(int indexValue, int saltValue)
{
    float laneNoise = StableNoise01(indexValue, saltValue);

    if (laneNoise < 0.3333f)
    {
        return -1;
    }

    if (laneNoise < 0.6666f)
    {
        return 0;
    }

    return 1;
}

static bool TextureSlotHasRealKey(const std::string& textureKey)
{
    return !textureKey.empty() && textureKey != "None";
}

static bool ShouldBorrowTextureForAlbedo(const SceneObject& sceneObject)
{
    //If albedo was never picked properly, let the regular texture slot do the heavy lifting instead of pretending it doesnt exist//
    return sceneObject.texture != nullptr &&
           TextureSlotHasRealKey(sceneObject.textureKey) &&
           (sceneObject.albedo == nullptr ||
            sceneObject.albedoKey == "None" ||
            (sceneObject.albedoKey == "Default" && sceneObject.textureKey != "Default"));
}

static int ParseTrailingInteger(const std::string& text)
{
    int characterIndex = (int)text.size() - 1;
    while (characterIndex >= 0 && std::isdigit((unsigned char)text[characterIndex]))
    {
        characterIndex--;
    }

    if (characterIndex == (int)text.size() - 1)
    {
        return -1;
    }

    return std::stoi(text.substr(characterIndex + 1));
}

static float DistanceBetween(const glm::vec3& firstPosition, const glm::vec3& secondPosition)
{
    glm::vec3 delta = firstPosition - secondPosition;
    return std::sqrt(delta.x * delta.x + delta.y * delta.y + delta.z * delta.z);
}

static glm::vec3 SafeNormalize(const glm::vec3& vectorValue, const glm::vec3& fallbackDirection)
{
    float vectorLength = glm::length(vectorValue);
    if (vectorLength <= 0.0001f)
    {
        return fallbackDirection;
    }

    return vectorValue / vectorLength;
}

void SplineShooterGame::BindAssets(Mesh* cubeMesh, Texture* defaultTexture)
{
    //Snag the shiny toys from engine land so gameplay doesnt throw invisible cubes at the player//
    cubeMesh_ = cubeMesh;
    defaultTexture_ = defaultTexture;
}

const char* SplineShooterGame::GetStateLabel() const
{
    switch (state_)
    {
        case GameState::Start:
            return "Start";
        case GameState::Playing:
            return "Playing";
        case GameState::Win:
            return "Win";
        case GameState::Lose:
            return "Lose";
        default:
            return "Unknown";
    }
}

void SplineShooterGame::SetLookDelta(float mouseDeltaX, float mouseDeltaY)
{
    //If spline mode is driving the bus we ignore mouse freelook so the ship doesnt have an identity crisis//
    if (state_ != GameState::Playing || controlMode_ != ControlMode::FreeFly)
    {
        return;
    }

    yaw_ += mouseDeltaX * mouseSensitivity_;
    pitch_ += mouseDeltaY * mouseSensitivity_;

    if (pitch_ > 89.0f)
    {
        pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f)
    {
        pitch_ = -89.0f;
    }
}

void SplineShooterGame::SyncViewAngles(float yawDegrees, float pitchDegrees)
{
    yaw_ = yawDegrees;
    pitch_ = pitchDegrees;

    if (pitch_ > 89.0f)
    {
        pitch_ = 89.0f;
    }
    if (pitch_ < -89.0f)
    {
        pitch_ = -89.0f;
    }
}

void SplineShooterGame::ToggleControlMode()
{
    //Little mode flip switch because one movement style is never enough drama for one project//
    if (controlMode_ == ControlMode::FreeFly)
    {
        controlMode_ = ControlMode::SplineFollow;
        splineCameraInitialized_ = false; //When we come back to the rail cam we re-seat it cleanly instead of dragging old freefly baggage around//
    }
    else
    {
        controlMode_ = ControlMode::FreeFly;
    }

    forwardInput_ = 0.0f;
}

void SplineShooterGame::SetPlayerShipMesh(Mesh* playerShipMesh, const std::string& playerShipMeshKey)
{
    //Cache the player ship setup here so Start() can slap it onto the player whenever play begins//
    playerShipMesh_ = playerShipMesh;
    playerShipMeshKey_ = playerShipMeshKey;
}

void SplineShooterGame::SetAsteroidMesh(Mesh* asteroidMesh, const std::string& asteroidMeshKey)
{
    //Same trick for the asteroid mesh because cube rocks only stopped being funny after the tenth one//
    asteroidMesh_ = asteroidMesh;
    asteroidMeshKey_ = asteroidMeshKey;
}

void SplineShooterGame::SetSecondaryAsteroidMesh(Mesh* asteroidMesh, const std::string& asteroidMeshKey)
{
    //Second rock buddy goes here so obstacle waves dont look like the same space potato copy-pasted forever//
    secondaryAsteroidMesh_ = asteroidMesh;
    secondaryAsteroidMeshKey_ = asteroidMeshKey;
}

void SplineShooterGame::SetPrimaryAsteroidMaterial(
    Texture* colorTexture,
    const std::string& colorTextureKey,
    Texture* specularTexture,
    const std::string& specularTextureKey)
{
    primaryAsteroidColorTexture_ = colorTexture;
    primaryAsteroidColorTextureKey_ = colorTextureKey;
    primaryAsteroidSpecularTexture_ = specularTexture;
    primaryAsteroidSpecularTextureKey_ = specularTextureKey;
}

void SplineShooterGame::SetSecondaryAsteroidMaterial(
    Texture* colorTexture,
    const std::string& colorTextureKey,
    Texture* specularTexture,
    const std::string& specularTextureKey)
{
    secondaryAsteroidColorTexture_ = colorTexture;
    secondaryAsteroidColorTextureKey_ = colorTextureKey;
    secondaryAsteroidSpecularTexture_ = specularTexture;
    secondaryAsteroidSpecularTextureKey_ = specularTextureKey;
}

void SplineShooterGame::SetBulletVisual(
    Mesh* bulletMesh,
    const std::string& bulletMeshKey,
    Texture* bulletTexture,
    const std::string& bulletTextureKey)
{
    bulletMesh_ = bulletMesh;
    bulletMeshKey_ = bulletMeshKey;
    bulletTexture_ = bulletTexture;
    bulletTextureKey_ = bulletTextureKey;
}

SceneObject* SplineShooterGame::FindObjectByName(Scene& scene, const std::string& objectName)
{
    for (auto& sceneObject : scene.GetObjects())
    {
        if (sceneObject.name == objectName)
        {
            return &sceneObject;
        }
    }

    return nullptr;
}

SceneObject* SplineShooterGame::FindObjectByEntity(Scene& scene, Entity entity)
{
    if (entity.Id == 0)
    {
        return nullptr;
    }

    for (auto& sceneObject : scene.GetObjects())
    {
        if (sceneObject.entity.Id == entity.Id)
        {
            return &sceneObject;
        }
    }

    return nullptr;
}

bool SplineShooterGame::DestroyObjectByEntity(Scene& scene, Entity entity)
{
    auto& sceneObjects = scene.GetObjects();
    for (size_t objectIndex = 0; objectIndex < sceneObjects.size(); ++objectIndex)
    {
        if (sceneObjects[objectIndex].entity.Id == entity.Id)
        {
            scene.DestroyObjectAt(objectIndex);
            return true;
        }
    }

    return false;
}

void SplineShooterGame::ClearRuntimeObjects(Scene& scene)
{
    //Clean bullets and rocks so the next round doesnt inherit old crimes//
    for (const auto& bullet : activeBullets_)
    {
        DestroyObjectByEntity(scene, bullet.entity);
    }

    for (const auto& obstacle : activeObstacles_)
    {
        DestroyObjectByEntity(scene, obstacle.entity);
    }

    activeBullets_.clear();
    activeObstacles_.clear();
}

SceneObject* SplineShooterGame::EnsurePlayerObject(Scene& scene)
{
    SceneObject* playerObject = FindObjectByEntity(scene, playerEntity_);
    bool createdFreshPlayerObject = false;

    if (!playerObject)
    {
        playerObject = FindObjectByName(scene, "Player");
    }

    if (!playerObject)
    {
        scene.CreateObject();
        playerObject = &scene.GetObjects().back();
        createdFreshPlayerObject = true;
    }

    if (!playerObject)
    {
        return nullptr;
    }

    Mesh* resolvedPlayerMesh = playerShipMesh_ ? playerShipMesh_ : cubeMesh_;
    std::string resolvedPlayerMeshKey = playerShipMesh_ ? playerShipMeshKey_ : "Cube";

    playerObject->name = "Player";
    playerObject->mesh.mesh = resolvedPlayerMesh;
    playerObject->meshKey = resolvedPlayerMeshKey;

    if (playerObject->texture == nullptr || !TextureSlotHasRealKey(playerObject->textureKey))
    {
        playerObject->texture = defaultTexture_;
        playerObject->textureKey = defaultTexture_ ? "Default" : "None";
    }

    if (createdFreshPlayerObject)
    {
        //Fresh player gets the boring starter material until you dress it up in the editor//
        playerObject->albedo = playerObject->texture;
        playerObject->albedoKey = playerObject->textureKey;
        playerObject->specular = playerObject->texture;
        playerObject->specularKey = playerObject->textureKey;
    }
    else
    {
        if (ShouldBorrowTextureForAlbedo(*playerObject))
        {
            playerObject->albedo = playerObject->texture;
            playerObject->albedoKey = playerObject->textureKey;
        }

        if (playerObject->specular == nullptr || playerObject->specularKey == "None")
        {
            playerObject->specular = playerObject->albedo ? playerObject->albedo : defaultTexture_;
            playerObject->specularKey = playerObject->albedo ? playerObject->albedoKey : "Default";
        }
    }

    playerObject->shininess = 64.0f;
    playerObject->transform.scale = glm::vec3(0.35f);

    playerEntity_ = playerObject->entity;
    return playerObject;
}

void SplineShooterGame::PlacePlayerAtTrackStart(Scene& scene, Camera& camera)
{
    SceneObject* playerObject = EnsurePlayerObject(scene);
    if (!playerObject)
    {
        return;
    }

    std::vector<glm::vec3> controlPoints = CollectSplineControlPoints(scene);
    if ((int)controlPoints.size() < 4)
    {
        return;
    }

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 trackCenterPosition = SampleCatmullRom(controlPoints, 0.0f);
    glm::vec3 playerPosition = trackCenterPosition;
    glm::vec3 forwardDirection = SafeNormalize(TangentCatmullRom(controlPoints, 0.0f), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 rightDirection = SafeNormalize(glm::cross(forwardDirection, worldUp), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 upDirection = SafeNormalize(glm::cross(rightDirection, forwardDirection), worldUp);

    forwardDirection_ = forwardDirection;
    rightDirection_ = rightDirection;
    upDirection_ = upDirection;

    playerObject->transform.position = playerPosition;
    UpdateSplineCameraRig(0.0f, controlPoints, 0.0f, trackCenterPosition, forwardDirection, camera, true);
}

void SplineShooterGame::ApplyAsteroidVisual(SceneObject& asteroidObject, bool useSecondaryVariant)
{
    Mesh* resolvedAsteroidMesh = asteroidMesh_ ? asteroidMesh_ : cubeMesh_;
    std::string resolvedAsteroidMeshKey = asteroidMesh_ ? asteroidMeshKey_ : "Cube";
    Texture* resolvedColorTexture = primaryAsteroidColorTexture_ ? primaryAsteroidColorTexture_ : defaultTexture_;
    std::string resolvedColorTextureKey = primaryAsteroidColorTexture_ ? primaryAsteroidColorTextureKey_ : "Default";
    Texture* resolvedSpecularTexture = primaryAsteroidSpecularTexture_ ? primaryAsteroidSpecularTexture_ : resolvedColorTexture;
    std::string resolvedSpecularTextureKey = primaryAsteroidSpecularTexture_ ? primaryAsteroidSpecularTextureKey_ : resolvedColorTextureKey;
    float resolvedShininess = 22.0f;
    glm::vec3 resolvedScale = glm::vec3(0.68f);

    if (useSecondaryVariant)
    {
        if (secondaryAsteroidMesh_ != nullptr)
        {
            resolvedAsteroidMesh = secondaryAsteroidMesh_;
            resolvedAsteroidMeshKey = secondaryAsteroidMeshKey_;
        }

        if (secondaryAsteroidColorTexture_ != nullptr)
        {
            resolvedColorTexture = secondaryAsteroidColorTexture_;
            resolvedColorTextureKey = secondaryAsteroidColorTextureKey_;
        }

        resolvedSpecularTexture =
            secondaryAsteroidSpecularTexture_ != nullptr
                ? secondaryAsteroidSpecularTexture_
                : resolvedColorTexture;
        resolvedSpecularTextureKey =
            secondaryAsteroidSpecularTexture_ != nullptr
                ? secondaryAsteroidSpecularTextureKey_
                : resolvedColorTextureKey;
        resolvedShininess = 34.0f;
        resolvedScale = glm::vec3(0.58f);
    }

    asteroidObject.mesh.mesh = resolvedAsteroidMesh;
    asteroidObject.meshKey = resolvedAsteroidMeshKey;
    asteroidObject.texture = resolvedColorTexture;
    asteroidObject.textureKey = resolvedColorTextureKey;
    asteroidObject.albedo = resolvedColorTexture;
    asteroidObject.albedoKey = resolvedColorTextureKey;
    asteroidObject.specular = resolvedSpecularTexture;
    asteroidObject.specularKey = resolvedSpecularTextureKey;
    asteroidObject.shininess = resolvedShininess;
    asteroidObject.transform.scale = resolvedScale;
}

void SplineShooterGame::UpdateSplineCameraRig(
    float deltaTime,
    const std::vector<glm::vec3>& controlPoints,
    float sampledSplineT,
    const glm::vec3& trackCenterPosition,
    const glm::vec3& forwardDirection,
    Camera& camera,
    bool snapImmediately)
{
    if ((int)controlPoints.size() < 4)
    {
        return;
    }

    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 flattenedForwardDirection =
        SafeNormalize(glm::vec3(forwardDirection.x, 0.0f, forwardDirection.z), glm::vec3(0.0f, 0.0f, -1.0f));
    (void)sampledSplineT;

    if (snapImmediately || !splineCameraInitialized_)
    {
        splineCameraRailOrigin_ = trackCenterPosition;
        splineCameraRailForward_ = flattenedForwardDirection;
        splineCameraRailRight_ =
            SafeNormalize(glm::cross(splineCameraRailForward_, worldUp), glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::vec3 trackDeltaFromCameraStart = trackCenterPosition - splineCameraRailOrigin_;
    float forwardProgressAlongRail = glm::dot(trackDeltaFromCameraStart, splineCameraRailForward_);
    float lateralProgressAlongRail = glm::dot(trackDeltaFromCameraStart, splineCameraRailRight_);
    float verticalProgressAlongRail = glm::dot(trackDeltaFromCameraStart, worldUp);

    glm::vec3 desiredCameraPosition =
        splineCameraRailOrigin_ +
        splineCameraRailForward_ * (forwardProgressAlongRail - followDistance_) +
        splineCameraRailRight_ * (lateralProgressAlongRail * cameraTrackLateralFollow_) +
        worldUp * (followHeight_ + verticalProgressAlongRail * cameraTrackVerticalFollow_);

    glm::vec3 desiredCameraTarget =
        splineCameraRailOrigin_ +
        splineCameraRailForward_ * (forwardProgressAlongRail + lookAheadDistance_ + cameraTrackLookAheadT_) +
        splineCameraRailRight_ * (lateralProgressAlongRail * cameraTrackLateralFollow_) +
        worldUp * (cameraAimHeight_ + verticalProgressAlongRail * cameraTrackVerticalFollow_);

    if (snapImmediately || !splineCameraInitialized_)
    {
        camera.position = desiredCameraPosition;
        splineCameraInitialized_ = true;
    }
    else
    {
        float followBlend = std::clamp(deltaTime * cameraFollowResponsiveness_, 0.0f, 1.0f);
        camera.position = glm::mix(camera.position, desiredCameraPosition, followBlend);
    }

    glm::vec3 cameraLookDirection = SafeNormalize(desiredCameraTarget - camera.position, splineCameraRailForward_);

    //The rail camera only moves forward now, so left-right spline wiggles stop dragging the whole view around like nonsense//
    camera.rotation.y = glm::degrees(std::atan2(cameraLookDirection.z, cameraLookDirection.x));
    camera.rotation.x = glm::degrees(std::asin(std::clamp(cameraLookDirection.y, -1.0f, 1.0f)));
}

std::vector<glm::vec3> SplineShooterGame::CollectSplineControlPoints(Scene& scene)
{
    std::vector<std::pair<int, glm::vec3>> indexedControlPoints;

    //Collect every point named like SplinePoint_X because naming conventions are cheaper than real tooling//
    for (const auto& sceneObject : scene.GetObjects())
    {
        if (!StartsWithPrefix(sceneObject.name, "SplinePoint_"))
        {
            continue;
        }

        int pointIndex = ParseTrailingInteger(sceneObject.name);
        if (pointIndex < 0)
        {
            continue;
        }

        indexedControlPoints.push_back({ pointIndex, sceneObject.transform.position });
    }

    std::sort(
        indexedControlPoints.begin(),
        indexedControlPoints.end(),
        [](const auto& leftPoint, const auto& rightPoint)
        {
            return leftPoint.first < rightPoint.first;
        });

    std::vector<glm::vec3> controlPoints;
    controlPoints.reserve(indexedControlPoints.size());
    for (const auto& indexedPoint : indexedControlPoints)
    {
        controlPoints.push_back(indexedPoint.second);
    }

    return controlPoints;
}

glm::vec3 SplineShooterGame::CatmullRom(
    const glm::vec3& previousPoint,
    const glm::vec3& startPoint,
    const glm::vec3& endPoint,
    const glm::vec3& nextPoint,
    float segmentAlpha)
{
    float alphaSquared = segmentAlpha * segmentAlpha;
    float alphaCubed = alphaSquared * segmentAlpha;

    return 0.5f * (
        (2.0f * startPoint) +
        (-previousPoint + endPoint) * segmentAlpha +
        (2.0f * previousPoint - 5.0f * startPoint + 4.0f * endPoint - nextPoint) * alphaSquared +
        (-previousPoint + 3.0f * startPoint - 3.0f * endPoint + nextPoint) * alphaCubed);
}

glm::vec3 SplineShooterGame::SampleCatmullRom(const std::vector<glm::vec3>& controlPoints, float splineT) const
{
    int segmentIndex = (int)std::floor(splineT);
    float segmentAlpha = splineT - (float)segmentIndex;
    int clampedControlPointIndex = std::clamp(segmentIndex, 0, (int)controlPoints.size() - 4);

    return CatmullRom(
        controlPoints[clampedControlPointIndex],
        controlPoints[clampedControlPointIndex + 1],
        controlPoints[clampedControlPointIndex + 2],
        controlPoints[clampedControlPointIndex + 3],
        segmentAlpha);
}

glm::vec3 SplineShooterGame::TangentCatmullRom(const std::vector<glm::vec3>& controlPoints, float splineT) const
{
    const float tangentSampleOffset = 0.01f;
    float maxSplineT = std::max(0.0f, (float)(controlPoints.size() - 3) - 0.001f);

    glm::vec3 startSample = SampleCatmullRom(controlPoints, std::max(0.0f, splineT - tangentSampleOffset));
    glm::vec3 endSample = SampleCatmullRom(controlPoints, std::min(splineT + tangentSampleOffset, maxSplineT));

    return endSample - startSample;
}

void SplineShooterGame::SpawnObstacles(Scene& scene, const std::vector<glm::vec3>& controlPoints)
{
    activeObstacles_.clear();
    nextObstacleIndex_ = 0;

    if ((int)controlPoints.size() < 4)
    {
        return;
    }

    float maxSplineT = (float)(controlPoints.size() - 3);
    if (maxSplineT <= 0.0f)
    {
        return;
    }

    const float obstacleSpacing = 0.46f;
    const float extraSpawnOffset = 0.17f;
    const float horizontalLaneDistance = 1.35f;
    const float verticalLaneDistance = 0.92f;
    const float tableAsteroidRightNudge = 0.45f;

    auto spawnObstacleAt = [&](float splineT, int patternSalt, bool forceTopOrBottom)
    {
        scene.CreateObject();
        auto& obstacleObject = scene.GetObjects().back();
        int obstacleNumber = nextObstacleIndex_++;
        bool useSecondaryVariant = ((obstacleNumber % 2) == 1);

        obstacleObject.name = "Obstacle_" + std::to_string(obstacleNumber);
        ApplyAsteroidVisual(obstacleObject, useSecondaryVariant);

        glm::vec3 obstaclePosition = SampleCatmullRom(controlPoints, splineT);
        glm::vec3 obstacleForwardDirection =
            SafeNormalize(TangentCatmullRom(controlPoints, splineT), glm::vec3(0.0f, 0.0f, -1.0f));
        glm::vec3 obstacleRightDirection =
            SafeNormalize(glm::cross(obstacleForwardDirection, glm::vec3(0.0f, 1.0f, 0.0f)), glm::vec3(1.0f, 0.0f, 0.0f));
        glm::vec3 obstacleUpDirection =
            SafeNormalize(glm::cross(obstacleRightDirection, obstacleForwardDirection), glm::vec3(0.0f, 1.0f, 0.0f));

        int horizontalLaneIndex = PickLaneIndex(obstacleNumber, patternSalt + 3);
        int verticalLaneIndex = PickLaneIndex(obstacleNumber, patternSalt + 11);

        if (forceTopOrBottom && verticalLaneIndex == 0)
        {
            verticalLaneIndex = StableNoise01(obstacleNumber, patternSalt + 17) > 0.5f ? 1 : -1;
        }

        float laneOffset = (float)horizontalLaneIndex * horizontalLaneDistance;
        float verticalOffset = (float)verticalLaneIndex * verticalLaneDistance;

        if (useSecondaryVariant)
        {
            //Table-texture rock gets nudged right a smidge so it stops camping dead-center all the time//
            laneOffset += tableAsteroidRightNudge;
        }

        obstaclePosition += obstacleRightDirection * laneOffset;
        obstaclePosition += obstacleUpDirection * verticalOffset;

        obstacleObject.transform.position = obstaclePosition;
        obstacleObject.transform.rotation.y = (float)(obstacleNumber * 37);
        activeObstacles_.push_back({
            obstacleObject.entity,
            useSecondaryVariant ? 0.52f : 0.62f,
            useSecondaryVariant ? 1 : 2,
            useSecondaryVariant ? secondaryAsteroidContactDamage_ : primaryAsteroidContactDamage_ });
    };

    //Spawn more chunky space pebbles now, including some top and bottom jerks so vertical dodging finally matters//
    int spawnBandIndex = 0;
    for (float splineT = 0.72f; splineT < maxSplineT - 0.15f; splineT += obstacleSpacing, ++spawnBandIndex)
    {
        bool mainSpawnWantsVerticalLane = StableNoise01(spawnBandIndex, 21) > 0.58f;
        spawnObstacleAt(splineT, spawnBandIndex * 5, mainSpawnWantsVerticalLane);

        if (StableNoise01(spawnBandIndex, 37) > 0.42f)
        {
            float extraSplineT = splineT + extraSpawnOffset;
            if (extraSplineT < maxSplineT - 0.12f)
            {
                spawnObstacleAt(extraSplineT, spawnBandIndex * 7 + 19, true);
            }
        }
    }
}

void SplineShooterGame::Start(Scene& scene)
{
    running_ = true;
    state_ = GameState::Start;
    health_ = 3;
    score_ = 0;
    controlMode_ = ControlMode::SplineFollow;
    splineSegmentT_ = 0.0f;
    horizontalStrafeInput_ = 0.0f;
    verticalStrafeInput_ = 0.0f;
    forwardInput_ = 0.0f;
    nextBulletIndex_ = 0;
    nextObstacleIndex_ = 0;
    splineCameraInitialized_ = false;
    splineCameraRailOrigin_ = glm::vec3(0.0f);
    splineCameraRailForward_ = glm::vec3(0.0f, 0.0f, -1.0f);
    splineCameraRailRight_ = glm::vec3(1.0f, 0.0f, 0.0f);

    ClearRuntimeObjects(scene);
    EnsurePlayerObject(scene); //Make sure the ship exists before the HUD starts bossing us around//
}

void SplineShooterGame::BeginRound(Scene& scene)
{
    if (!running_)
    {
        Start(scene);
    }

    ClearRuntimeObjects(scene);

    health_ = 3;
    score_ = 0;
    splineSegmentT_ = 0.0f;
    horizontalStrafeInput_ = 0.0f;
    verticalStrafeInput_ = 0.0f;
    forwardInput_ = 0.0f;
    nextBulletIndex_ = 0;
    nextObstacleIndex_ = 0;
    controlMode_ = ControlMode::SplineFollow; //Every fresh round starts on the actual assignment track, imagine that//
    splineCameraInitialized_ = false;
    splineCameraRailOrigin_ = glm::vec3(0.0f);
    splineCameraRailForward_ = glm::vec3(0.0f, 0.0f, -1.0f);
    splineCameraRailRight_ = glm::vec3(1.0f, 0.0f, 0.0f);

    std::vector<glm::vec3> controlPoints = CollectSplineControlPoints(scene);
    if ((int)controlPoints.size() < 4)
    {
        state_ = GameState::Start;
        return;
    }

    EnsurePlayerObject(scene);
    SpawnObstacles(scene, controlPoints);
    state_ = GameState::Playing;
}

void SplineShooterGame::Stop(Scene& scene)
{
    running_ = false;
    state_ = GameState::Start;
    forwardInput_ = 0.0f;
    horizontalStrafeInput_ = 0.0f;
    verticalStrafeInput_ = 0.0f;
    splineCameraInitialized_ = false;
    splineCameraRailOrigin_ = glm::vec3(0.0f);
    splineCameraRailForward_ = glm::vec3(0.0f, 0.0f, -1.0f);
    splineCameraRailRight_ = glm::vec3(1.0f, 0.0f, 0.0f);

    ClearRuntimeObjects(scene);
}

void SplineShooterGame::Shoot(Scene& scene)
{
    Mesh* resolvedBulletMesh = bulletMesh_ ? bulletMesh_ : cubeMesh_;
    Texture* resolvedBulletTexture = bulletTexture_ ? bulletTexture_ : defaultTexture_;
    std::string resolvedBulletMeshKey = bulletMesh_ ? bulletMeshKey_ : "Cube";
    std::string resolvedBulletTextureKey = bulletTexture_ ? bulletTextureKey_ : "Default";

    if (!running_ || state_ != GameState::Playing || resolvedBulletMesh == nullptr)
    {
        return;
    }

    SceneObject* playerObject = FindObjectByEntity(scene, playerEntity_);
    if (!playerObject)
    {
        playerObject = FindObjectByName(scene, "Player");
    }

    if (!playerObject)
    {
        return;
    }

    scene.CreateObject();
    auto& bulletObject = scene.GetObjects().back();
    int bulletNumber = nextBulletIndex_++;

    //Bullet is round-ish now so it stops looking like we are firing tiny angry bricks//
    bulletObject.name = "Bullet_" + std::to_string(bulletNumber);
    bulletObject.mesh.mesh = resolvedBulletMesh;
    bulletObject.meshKey = resolvedBulletMeshKey;

    bulletObject.texture = resolvedBulletTexture;
    bulletObject.textureKey = resolvedBulletTextureKey;
    bulletObject.albedo = resolvedBulletTexture;
    bulletObject.albedoKey = resolvedBulletTextureKey;
    bulletObject.specular = resolvedBulletTexture;
    bulletObject.specularKey = resolvedBulletTextureKey;
    bulletObject.shininess = 20.0f;

    bulletObject.transform.scale = glm::vec3(0.09f);
    bulletObject.transform.position = playerObject->transform.position + forwardDirection_ * 0.7f;

    Bullet newBullet;
    newBullet.entity = bulletObject.entity;
    newBullet.velocity = forwardDirection_ * 30.0f; //Bit faster again so shooting feels punchy even after slowing the ship down a ton//
    newBullet.remainingLife = 2.0f;
    newBullet.radius = 0.15f;

    activeBullets_.push_back(newBullet);
}

void SplineShooterGame::UpdateFreeFly(float deltaTime, SceneObject& playerObject, Camera& camera)
{
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);

    //Turn yaw/pitch into a forward vector the old fashioned way because trigonometry yearns for suffering//
    float yawRadians = glm::radians(yaw_);
    float pitchRadians = glm::radians(pitch_);

    glm::vec3 forwardDirection;
    forwardDirection.x = cosf(yawRadians) * cosf(pitchRadians);
    forwardDirection.y = sinf(pitchRadians);
    forwardDirection.z = sinf(yawRadians) * cosf(pitchRadians);
    forwardDirection = SafeNormalize(forwardDirection, glm::vec3(0.0f, 0.0f, -1.0f));

    glm::vec3 rightDirection = SafeNormalize(glm::cross(forwardDirection, worldUp), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 upDirection = SafeNormalize(glm::cross(rightDirection, forwardDirection), worldUp);

    //Store the latest basis so bullets leave the ship in the right spicy direction//
    forwardDirection_ = forwardDirection;
    rightDirection_ = rightDirection;
    upDirection_ = upDirection;

    float horizontalStrafe = std::clamp(horizontalStrafeInput_, -1.0f, 1.0f);
    float verticalStrafe = std::clamp(verticalStrafeInput_, -1.0f, 1.0f);
    float forwardMove = std::clamp(forwardInput_, -1.0f, 1.0f);

    glm::vec3 movementDirection =
        forwardDirection * forwardMove +
        rightDirection * horizontalStrafe +
        upDirection * verticalStrafe;

    //No normalization here on purpose so diagonal boost is part of the jank package for now//
    playerObject.transform.position += movementDirection * moveSpeed_ * deltaTime;

    camera.position = playerObject.transform.position - forwardDirection * 4.0f + worldUp * 1.5f;
    camera.rotation.x = pitch_;
    camera.rotation.y = yaw_;
}

void SplineShooterGame::UpdateSplineFollow(float deltaTime, SceneObject& playerObject, Scene& scene, Camera& camera)
{
    std::vector<glm::vec3> controlPoints = CollectSplineControlPoints(scene);
    if ((int)controlPoints.size() < 4)
    {
        state_ = GameState::Start;
        return;
    }

    float maxSplineT = (float)(controlPoints.size() - 3);
    if (maxSplineT <= 0.0f)
    {
        state_ = GameState::Start;
        return;
    }

    splineSegmentT_ += splineSpeed_ * deltaTime;
    if (splineSegmentT_ >= maxSplineT)
    {
        splineSegmentT_ = std::max(0.0f, maxSplineT - 0.001f);
        state_ = GameState::Win;
        ClearRuntimeObjects(scene);
    }

    float sampledSplineT = std::max(0.0f, std::min(splineSegmentT_, maxSplineT - 0.001f));
    glm::vec3 trackCenterPosition = SampleCatmullRom(controlPoints, sampledSplineT);
    glm::vec3 playerPosition = trackCenterPosition;
    glm::vec3 worldUp(0.0f, 1.0f, 0.0f);
    glm::vec3 forwardDirection =
        SafeNormalize(TangentCatmullRom(controlPoints, sampledSplineT), glm::vec3(0.0f, 0.0f, -1.0f));
    glm::vec3 rightDirection =
        SafeNormalize(glm::cross(forwardDirection, worldUp), glm::vec3(1.0f, 0.0f, 0.0f));
    glm::vec3 upDirection =
        SafeNormalize(glm::cross(rightDirection, forwardDirection), worldUp);

    //Store the latest spline basis so shooting and strafing dont drift into clown town//
    forwardDirection_ = forwardDirection;
    rightDirection_ = rightDirection;
    upDirection_ = upDirection;

    float horizontalStrafe = std::clamp(horizontalStrafeInput_, -1.0f, 1.0f);
    float verticalStrafe = std::clamp(verticalStrafeInput_, -1.0f, 1.0f);

    playerPosition += rightDirection * horizontalStrafe * laneHalfWidth_;
    playerPosition += upDirection * verticalStrafe * (laneHalfWidth_ * 0.6f);

    playerObject.transform.position = playerPosition;
    UpdateSplineCameraRig(deltaTime, controlPoints, sampledSplineT, trackCenterPosition, forwardDirection, camera, false);
}

void SplineShooterGame::UpdateBullets(float deltaTime, Scene& scene)
{
    //Walk backwards through the bullet list so erasing stuff doesnt turn the loop into soup//
    for (int bulletIndex = (int)activeBullets_.size() - 1; bulletIndex >= 0; --bulletIndex)
    {
        activeBullets_[bulletIndex].remainingLife -= deltaTime;

        SceneObject* bulletObject = FindObjectByEntity(scene, activeBullets_[bulletIndex].entity);
        if (!bulletObject || activeBullets_[bulletIndex].remainingLife <= 0.0f)
        {
            if (bulletObject)
            {
                DestroyObjectByEntity(scene, activeBullets_[bulletIndex].entity);
            }
            activeBullets_.erase(activeBullets_.begin() + bulletIndex);
            continue;
        }

        bulletObject->transform.position += activeBullets_[bulletIndex].velocity * deltaTime;
    }
}

void SplineShooterGame::CheckCollisions(Scene& scene)
{
    if (state_ != GameState::Playing)
    {
        return;
    }

    SceneObject* playerObject = FindObjectByEntity(scene, playerEntity_);
    if (!playerObject)
    {
        playerObject = FindObjectByName(scene, "Player");
    }

    if (!playerObject)
    {
        return;
    }

    //Player vs Obstacles because somebody has to be the victim in this soap opera//
    for (int obstacleIndex = (int)activeObstacles_.size() - 1; obstacleIndex >= 0; --obstacleIndex)
    {
        SceneObject* obstacleObject = FindObjectByEntity(scene, activeObstacles_[obstacleIndex].entity);
        if (!obstacleObject)
        {
            activeObstacles_.erase(activeObstacles_.begin() + obstacleIndex);
            continue;
        }

        float distanceToObstacle = DistanceBetween(playerObject->transform.position, obstacleObject->transform.position);
        if (distanceToObstacle < (0.45f + activeObstacles_[obstacleIndex].radius))
        {
            health_ -= activeObstacles_[obstacleIndex].contactDamage;
            DestroyObjectByEntity(scene, activeObstacles_[obstacleIndex].entity);
            activeObstacles_.erase(activeObstacles_.begin() + obstacleIndex);

            if (health_ <= 0)
            {
                health_ = 0;
                state_ = GameState::Lose;
                ClearRuntimeObjects(scene);
                return;
            }
        }
    }

    //Bullets vs Obstacles because diplomacy has failed and now we are doing cube violence//
    for (int bulletIndex = (int)activeBullets_.size() - 1; bulletIndex >= 0; --bulletIndex)
    {
        SceneObject* bulletObject = FindObjectByEntity(scene, activeBullets_[bulletIndex].entity);
        if (!bulletObject)
        {
            activeBullets_.erase(activeBullets_.begin() + bulletIndex);
            continue;
        }

        bool bulletWasConsumed = false;

        for (int obstacleIndex = (int)activeObstacles_.size() - 1; obstacleIndex >= 0; --obstacleIndex)
        {
            SceneObject* obstacleObject = FindObjectByEntity(scene, activeObstacles_[obstacleIndex].entity);
            if (!obstacleObject)
            {
                continue;
            }

            float distanceToObstacle = DistanceBetween(bulletObject->transform.position, obstacleObject->transform.position);
            if (distanceToObstacle < (activeBullets_[bulletIndex].radius + activeObstacles_[obstacleIndex].radius))
            {
                activeObstacles_[obstacleIndex].hitPoints -= 1;

                DestroyObjectByEntity(scene, activeBullets_[bulletIndex].entity);
                activeBullets_.erase(activeBullets_.begin() + bulletIndex);
                bulletWasConsumed = true;

                if (activeObstacles_[obstacleIndex].hitPoints <= 0)
                {
                    DestroyObjectByEntity(scene, activeObstacles_[obstacleIndex].entity);
                    activeObstacles_.erase(activeObstacles_.begin() + obstacleIndex);
                    score_ += 10;
                }
                break;
            }
        }

        if (bulletWasConsumed)
        {
            continue;
        }
    }
}

void SplineShooterGame::Update(float deltaTime, Scene& scene, Camera& camera)
{
    if (!running_)
    {
        return;
    }

    SceneObject* playerObject = EnsurePlayerObject(scene);
    if (!playerObject)
    {
        return;
    }

    if (state_ == GameState::Start)
    {
        PlacePlayerAtTrackStart(scene, camera);
        return;
    }

    if (state_ != GameState::Playing)
    {
        return;
    }

    if (controlMode_ == ControlMode::FreeFly)
    {
        UpdateFreeFly(deltaTime, *playerObject, camera);
    }
    else
    {
        UpdateSplineFollow(deltaTime, *playerObject, scene, camera);
    }

    if (state_ != GameState::Playing)
    {
        return;
    }

    UpdateBullets(deltaTime, scene);
    CheckCollisions(scene);
}
