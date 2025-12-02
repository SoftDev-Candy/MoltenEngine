//
// Created by Candy on 11/18/2025.
//

#ifndef B_WENGINE_RENDERABLE_HPP
#define B_WENGINE_RENDERABLE_HPP
#include "glm/ext/matrix_transform.hpp"
class Renderable
{
public:
    virtual void Render(glm::mat4& mvp) = 0;
    virtual ~Renderable() = default;

};


#endif //B_WENGINE_RENDERABLE_HPP