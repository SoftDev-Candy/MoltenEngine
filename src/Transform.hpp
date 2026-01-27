//
// Created by Candy on 11/27/2025.
//

#ifndef B_WENGINE_TRANSFORM_HPP
#define B_WENGINE_TRANSFORM_HPP

#include<glm/glm.hpp>

#include "glm/ext/matrix_transform.hpp"

struct Transform
{
glm::vec3 position {0.0f,0.0f,0.0f};
glm::vec3 rotation{0.0f,0.0f,0.0f}; // DEGREES (editor-friendly)
glm::vec3 scale {1.0f,1.0f,1.0f};

glm::mat4 GetMatrix()const
{
    glm::mat4 model = glm::mat4(1.0f);

    //1.Translating it hoe make it feel bonita  //
    model = glm::translate(model,position);

    //2.Rotate(XYZ) (convert degrees -> radians)
    model = glm::rotate(model, glm::radians(rotation.x), glm::vec3(1,0,0));
    model = glm::rotate(model, glm::radians(rotation.y), glm::vec3(0,1,0));
    model = glm::rotate(model, glm::radians(rotation.z), glm::vec3(0,0,1));

    //3.If she ain't fat she ain't right//
    model = glm::scale(model , scale);

    return model;
}
};

#endif //B_WENGINE_TRANSFORM_HPP