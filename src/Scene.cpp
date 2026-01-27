//
// Created by Candy on 11/20/2025.
//

#include "Scene.hpp"


std::vector<Light>& GetLights() { return lights; }
const std::vector<Light>& GetLights() const { return lights; }

int AddLight()
{
    lights.push_back(Light{});
    return (int)lights.size() - 1;
}

bool RemoveLight(int index)
{
    if (index < 0 || index >= (int)lights.size()) return false;
    lights.erase(lights.begin() + index);
    return true;
}
