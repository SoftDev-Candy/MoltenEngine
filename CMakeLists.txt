cmake_minimum_required(VERSION 4.0)
project(B_WEngine)

set(CMAKE_CXX_STANDARD 20)

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
        src/Transform.cpp
        src/Transform.hpp
        src/Entity.cpp
        src/Entity.hpp
        src/Camera.cpp
        src/Camera.hpp
        external/imgui/imgui.cpp
        external/imgui/imgui_draw.cpp
        external/imgui/imgui_widgets.cpp
        external/imgui/imgui_tables.cpp
        external/imgui/imgui_impl_glfw.cpp
        external/imgui/imgui_impl_opengl3.cpp
        external/imgui/imgui_demo.cpp
        external/stb/stb_image.h
        src/Texture.cpp
        src/Texture.hpp
        src/Mesh.cpp
        src/Mesh.hpp
        src/MeshComponent.cpp
        src/MeshComponent.hpp
        src/ShaderManager.cpp
        src/ShaderManager.hpp
        src/ui/EditorStyle.cpp
        src/ui/EditorStyle.hpp
        src/ui/EditorWidgets.cpp
        src/ui/EditorWidgets.hpp
        src/ui/UIManager.cpp
        src/ui/UIManager.hpp
        src/MeshManager.cpp
        src/MeshManager.hpp
        src/ObjLoader.cpp
        src/ObjLoader.hpp
        src/TextureManager.cpp
        src/TextureManager.hpp
        src/Message/Message.cpp
        src/Message/Message.hpp
        src/message/FileDroppedMessage.cpp
        src/message/FileDroppedMessage.hpp
        src/message/MessageQueue.cpp
        src/message/MessageQueue.hpp
        src/message/ImportMeshMessage.cpp
        src/message/ImportMeshMessage.hpp
        src/message/ImportTextureMessage.cpp
        src/message/ImportTextureMessage.hpp
        src/message/CreateEntityMessage.cpp
        src/message/CreateEntityMessage.hpp
        src/message/DeleteEntityMessage.cpp
        src/message/DeleteEntityMessage.hpp
        src/message/SetEntityMeshMessage.cpp
        src/message/SetEntityMeshMessage.hpp

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
        external/imgui
        external/stb
    )

#Had an error so basically need to tell the linker its a console app and we are not going to be using winmain() but just main()
set_target_properties(B_WEngine PROPERTIES WIN32_EXECUTABLE OFF)