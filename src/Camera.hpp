//
// Created by Candy on 12/1/2025.
//

#ifndef B_WENGINE_CAMERA_HPP
#define B_WENGINE_CAMERA_HPP
#include "glm/glm.hpp"
#include "glm/ext/matrix_clip_space.hpp"
#include "glm/ext/matrix_transform.hpp"

class Camera
{
public:
    glm::vec3 position{0.0f,0.0f,3.0f};
    glm::vec3 target  {0.0f,0.0f,0.0f};
    glm::vec3 up      {0.0f,1.0f,0.0f};

    glm::mat4 GetView()const
    {
        return glm::lookAt(position,target,up);
    }

    glm::mat4 GetProjection(float width , float height)const
    {
        return glm::perspective(glm::radians(this->fov),
                                        width / height,
                                        0.1f,
                                        100.0f);
    }

    //Camera Viewing angles //
    float fov = 45.0f;

};


#endif //B_WENGINE_CAMERA_HPP