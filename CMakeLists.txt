cmake_minimum_required(VERSION 4.0)
project(B_WEngine)

set(CMAKE_CXX_STANDARD 14)

#Adding GLFW from source

add_subdirectory(external/glfw)

add_executable(B_WEngine src/main.cpp)

find_package(OpenGL REQUIRED)

target_link_libraries(B_WEngine PRIVATE OpenGL::GL glfw)

#Had an error so basically need to tell the linker its a console app and we are not going to be using winmain() but just main()
set_target_properties(B_WEngine PROPERTIES WIN32_EXECUTABLE OFF)