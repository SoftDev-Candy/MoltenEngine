//
// Created by Candy on 1/4/2026.
//

#ifndef B_WENGINE_OBJLOADER_HPP
#define B_WENGINE_OBJLOADER_HPP

#include<vector>
#include<iostream>

//Option were to make a pair or a struct but I think
//I also do need to work with faces and normals so struct is a better design choice//

struct ObjMeshData{

    std::vector<float>vertices;
    std::vector<unsigned int>indices;

    bool hasUVs = false;
    bool hasNormals = false;

};

ObjMeshData LoadOBJ(const std::string &path , bool isFlipped =false);


#endif //B_WENGINE_OBJLOADER_HPP