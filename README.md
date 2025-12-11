# MoltenEngine
A basic C++ engine using OpenGL and GLFW built with CMake and CLion.

## Build

**For windows Users**
```
mkdir B_WEngine
git clone https://github.com/SoftDev-Candy/MoltenEngine.git
mkdir build
cd build
cmake -G "Xcode" ..
cmake --build . --config Release.
```

**For Mac Users**
```
mkdir B_WEngine
git clone https://github.com/SoftDev-Candy/MoltenEngine.git
cd B_WEngine
mkdir build && cd build
cmake ..
cmake --build .
```

## Currently completed

**✔️ EngineContext**
window creation
update
render loop
manages the scene
manages renderer
camera

**✔️ Scene**
holds objects
each object has:
Entity ID
Transform
Renderable*

**✔️ Renderer**
begins frame
clears buffers
renders each object (but missing MVP)

**✔️ Renderable base class**
polymorphic Render()

**✔️ Triangle + Cube (mesh objects)**
VAO/VBO/EBO
Shader
can render themselves

**✔️ Shader system**
compile + link
bind
setMat4

**✔️ ShaderSource**
separate file for GLSL strings

**✔️ Transform**
position/rotation/scale
computes model matrix

✔️ GLAD + GLFW + GLM correctly installed
✔️ Depth test on
✔️ A rotating cube already works


## Current Progress 


