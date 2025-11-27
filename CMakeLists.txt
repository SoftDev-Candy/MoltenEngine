cmake_minimum_required(VERSION 4.0)
project(B_WEngine)

set(CMAKE_CXX_STANDARD 14)

#Adding GLFW from source

add_subdirectory(external/glfw)
add_subdirectory(external/glad)

add_executable(B_WEngine src/main.cpp
        src/Shader.cpp
        src/Shader.hpp
        src/Renderable.cpp
        src/Renderable.hpp
        src/Triangle.cpp
        src/Triangle.hpp
        src/Scene.cpp
        src/Scene.hpp
        src/ShaderSource.cpp
        src/ShaderSource.hpp
        src/Renderer.cpp
        src/Renderer.hpp
        src/EngineContext.cpp
        src/EngineContext.hpp
        src/Cube.cpp
        src/Cube.hpp
)

find_package(OpenGL REQUIRED)

target_link_libraries( B_WEngine PRIVATE
        OpenGL::GL
        glfw
        glad
)

target_include_directories(B_WEngine PRIVATE
        external/glad/include
        external/glfw/include
        external/glm
)

#Had an error so basically need to tell the linker its a console app and we are not going to be using winmain() but just main()
set_target_properties(B_WEngine PROPERTIES WIN32_EXECUTABLE OFF)