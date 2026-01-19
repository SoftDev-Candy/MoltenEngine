//
// Created by Candy on 12/1/2025.
//

#ifndef B_WENGINE_CAMERA_HPP
#define B_WENGINE_CAMERA_HPP
#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"
#include "glm/gtc/constants.hpp" // pi

class Camera
{
public:
    //Camera State//
    glm::vec3 position{0.0f, 0.0f, 3.0f};

    //Rotation in DEGREES (simple editor friendly)
    //x = pitch (look up/down), y = yaw (look left/right), z = roll (we ignore for now)
    glm::vec3 rotation{0.0f, -90.0f, 0.0f}; // yaw -90 so it looks toward -Z by default

    glm::vec3 up{0.0f, 1.0f, 0.0f};

    //Camera Viewing angles //
    float fov = 45.0f;

public:
    glm::mat4 GetView() const
    {
        //Convert yaw/pitch to forward direction//
        float yaw   = glm::radians(rotation.y);
        float pitch = glm::radians(rotation.x);

        glm::vec3 forward;
        forward.x = cosf(yaw) * cosf(pitch);
        forward.y = sinf(pitch);
        forward.z = sinf(yaw) * cosf(pitch);

        glm::vec3 target = position + glm::normalize(forward);

        return glm::lookAt(position, target, up);
    }

    glm::mat4 GetProjection(float width, float height) const
    {
        //Safety so we dont divide by 0 if window is weird//
        if (height <= 0.0f) height = 1.0f;

        return glm::perspective(glm::radians(fov),
                                width / height,
                                0.1f,
                                100.0f);
    }

};


#endif //B_WENGINE_CAMERA_HPP