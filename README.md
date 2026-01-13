# MoltenEngine
A basic **C++20** engine using **OpenGL + GLFW**, built with **CMake** (developed in **CLion**).  
Goal: a clean learning engine that grows toward an **editor-style workflow** (Scene/Renderer/UI/Assets).

---

## Tech Stack
- **C++ 20**, **OpenGL 3.3 Core**
- **GLFW**, **GLAD**, **GLM**
- Build: **CMake**
- IDE: **CLion** (Windows: MinGW or MSVC)
- Linux - Woring on it will(Update Soon)

---

## Build

### Windows (CLion + MinGW or Ninja)
```
git clone https://github.com/SoftDev-Candy/MoltenEngine.git
cd MoltenEngine
cmake -S . -B build
cmake --build build

```
If you want to force a generator:
MinGW Makefiles
```
cmake -S . -B build -G "MinGW Makefiles"
cmake --build build
```
Ninja
```
cmake -S . -B build -G Ninja
cmake --build build
macOS
bash
Copy code
git clone https://github.com/SoftDev-Candy/MoltenEngine.git
cd MoltenEngine
mkdir build && cd build
cmake ..
cmake --build .
```

## Currently Completed ✅
✔ EngineContext
Window creation
Update + render loop
Manages Scene
Manages Renderer
Camera (basic integration)

✔ Scene
Holds objects/entities. Each object currently has:
Entity ID
Transform
Renderable* (pointer/reference to something drawable)

✔ Renderer
Begins frame
Clears buffers
Renders each object
Note: MVP wiring is planned/partially missing depending on your latest branch.

✔ Renderable base class
Polymorphic Render() API

✔ Triangle + Cube (mesh objects)
VAO / VBO / EBO
Shader per object (current approach)
Can render themselves

✔ Shader system
Compile + link
Bind/use
setMat4 uniform support

✔ ShaderSource
Separate file for GLSL strings

✔ Transform
Position / Rotation / Scale
Computes Model matrix

✔ Dependencies / Rendering State
GLAD + GLFW + GLM installed correctly
Depth test enabled
A rotating cube works

## Current Progress (Editor Engine Direction)
# **Rendering (Next Milestones)**  

Full MVP pipeline: Model * View * Projection per entity
 Render multiple entities reliably (Scene-driven)
 Materials/texturing (sampler2D + UVs) (WIP)

# **Assets / Resources (Planned)**

 Texture loading via stb_image (RGB/RGBA + mipmaps)
 OBJ loading (positions + faces; UVs when present)
 MeshManager caching (store/reuse meshes by string key)
 ShaderManager caching (compile once, reuse safely)

# **Editor UI (Planned: ImGui)**

# **Dockspace layout** 

Scene Hierarchy (select entities)

# Inspector:
 Rename entity
 Edit Transform (Position/Rotation/Scale)
 Camera panel (FOV slider)
 Custom theme + widgets (molten/lava style)
 
# Controls (Planned / When Camera Input Exists)
Typical editor camera controls (if/when enabled):
W / S → forward / back
A / D → left / right
Q / E → down / up
UI → edit entity transforms + camera settings

## Screesnshots - 
Editor Overview
Inspector / Transform Editing
Rendering (Cube / Scene)

TODO- ADD images here 

## Project Structure (Target Shape)
```
src/
  EngineContext.*      # init + main loop orchestration
  Renderer.*           # draw loop + (MVP) setup
  Scene.*              # entity container
  Entity.*             # entity ID / handle (optional split)
  Transform.*          # position/rotation/scale -> model matrix
  Renderable.*         # polymorphic draw API

  Mesh.*               # VAO/VBO/EBO wrapper (planned)
  MeshManager.*        # mesh caching (planned)
  ObjLoader.*          # loads .obj -> vertices/indices (planned)

  Shader.*             # shader compile/link + uniforms
  ShaderManager.*      # shader caching (planned)

  Texture.*            # stb_image -> OpenGL texture (planned)

  ui/                  # editor ui (planned)
    UIManager.*        # hierarchy/inspector/camera panels
    EditorStyle.*      # theme
    EditorWidgets.*    # custom widgets
assets/
  textures/
  models/
docs/
  screenshots/

```
  
## Architecture Highlights (High-Level)
**Data Flow**

EngineContext initializes window + OpenGL
Scene + resources are created/loaded

Each frame:
Poll input
Update scene
Render scene via Renderer

**Ownership Rules (Simple + Safe)**
Planned direction:
Managers own resources (Mesh/Shader/Texture)
Entities reference assets via pointers/IDs/keys
Scene owns entity list and transforms
Roadmap / Course Requirements Checklist
Must-have
 UI framework (ImGui)
 Entity rendering + transforms (MVP)
 OBJ loader
 Mesh caching (MeshManager)
 Camera + FOV UI
 Texture loading (stb_image)
 Set mesh per entity (UI-driven)
 Set texture per entity (UI-driven)
 Drag & drop assets (ImGui payload)
 
 ## Short architecture document/report
 Message base class + subclasses
 Message queue
 Memory checking utilities

# Nice-to-have(Add-ons after inital setup)
 Scene serialization
 Better resource pipeline
 Triangulation / quad handling
 Thread/mutex demos

## Credits
Built as a FutureGames course engine project (renderer + asset + UI fundamentals).

css
Copy code
