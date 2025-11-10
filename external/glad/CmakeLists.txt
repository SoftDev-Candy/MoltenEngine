cmake_minimum_required(VERSION 3.16)
project(glad)

add_library(glad STATIC src/glad.c)

#include path here because the compiler doesn't really know how to find it
target_include_directories(glad
        PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}/include
        PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/include

)

