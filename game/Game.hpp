//
// Created by Candy on 2/12/2026.
//

#ifndef B_WENGINE_GAME_HPP
#define B_WENGINE_GAME_HPP

#include <glm/glm.hpp>
#include <vector>
#include <string>
class Scene;
class Camera;
struct SceneObject;

class SplineShooterGame
{
public:
    void Start(Scene& scene);
    void Stop();
    void Update(float dt, Scene& scene, Camera& cam);

    void SetInput(float strafeX, float strafeY) { strafeX_ = strafeX; strafeY_ = strafeY; }
    bool IsRunning() const { return running_; }

private:
    bool running_ = false;

    float seg_t = 0.0f;          // spline parameter (segments)
    float speed_ = 1.5f;      // segments per second (tune later)

    float strafeX_ = 0.0f;    // left/right
    float strafeY_ = 0.0f;    // up/down (optional)

    float followDist_   = 4.0f;
    float followHeight_ = 2.0f;
    float lookAhead_    = 2.0f;

private:
    SceneObject* FindByName(Scene& scene, const std::string& name);
    std::vector<glm::vec3> CollectSplinePoints(Scene& scene);

    glm::vec3 SampleCatmullRom(const std::vector<glm::vec3>& pts, float t) const;
    glm::vec3 TangentCatmullRom(const std::vector<glm::vec3>& pts, float t) const;

    static glm::vec3 CatmullRom(const glm::vec3& p0, const glm::vec3& p1,
                                const glm::vec3& p2, const glm::vec3& p3, float u);
};

#endif //B_WENGINE_GAME_HPP