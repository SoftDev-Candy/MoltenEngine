# **🌋MoltenEngine🌋 — Architecture Document**

> **What this is:** MoltenEngine is a small C++ editor-style engine prototype built to prove an end-to-end pipeline:  
> **Scene → Entity data → Renderer (OpenGL) → Assets (OBJ + textures) → Editor UI (ImGui) → Commands (MessageQueue)**

Note : This document is not final and is subject to change as the project progresses. 
---

## Contents

| # | Section | What it covers |
|---:|---|---|
| 1 | [Why MoltenEngine exists](#1-why-moltenengine-exists) | Goals, constraints, and design rules |
| 2 | [Core loop](#2-core-loop-what-happens-every-frame) | Init → Update → UI → Messages → Render |
| 3 | [Project structure map](#3-project-structure-map) | Folder + file responsibilities |
| 4 | [Scene and entity model](#4-the-data-model-scene--objects--components) | `SceneObject`, selection, deletion, keys |
| 5 | [Rendering pipeline](#5-rendering-pipeline-mvp--textures) | MVP, shader binding, per-entity textures |
| 6 | [Asset pipeline](#6-asset-pipeline-mesh--texture--obj) | Mesh/Texture managers + OBJ import |
| 7 | [Editor UI](#7-editor-ui-hierarchy--inspector--assets) | Hierarchy + Inspector + Assets + drag/drop |
| 8 | [Message system](#8-message-system-ui-asks-engine-does) | Decoupling UI actions from engine state |
| 9 | [Lifetime and shutdown](#9-lifetime--shutdown-avoiding-leaks-and-gl-traps) | Clearing managers before GL context dies |
| 10 | [Showcase snippets](#10-where-to-showcase-code-snippets-proudly) | Where to paste “proud code” blocks |
| 11 | [Limitations and next steps](#11-known-limitations--next-steps) | Honest gaps + logical future upgrades |

---

## 1. Why MoltenEngine exists

Most beginner engines become unmaintainable because everything gets wired together too early:
- UI directly edits engine state everywhere
- Renderer owns the scene or controls gameplay logic
- Assets are raw pointers floating around without ownership
- Importing meshes/textures becomes copy-paste chaos

MoltenEngine is built to **avoid that trap** while still shipping features quickly.

### Guiding design rules
- **Scene owns state** (entities, transforms, mesh assignments, texture assignments)
- **Renderer reads state** (does not own objects)
- **Managers own lifetimes** (MeshManager / TextureManager)
- **UI only expresses intent** (through MessageQueue)
- **EngineContext is the orchestrator** (glue layer + execution authority)

**Entry points:**
- [`MoltenEngine/src/main.cpp`](MoltenEngine/src/main.cpp)
- [`MoltenEngine/src/EngineContext.cpp`](MoltenEngine/src/EngineContext.cpp) / [`MoltenEngine/src/EngineContext.hpp`](MoltenEngine/src/EngineContext.hpp)

---

## 2. Core loop (what happens every frame)

MoltenEngine uses an editor-style frame loop:

### Init (one time)
- Create window (GLFW)
- Load OpenGL functions (GLAD)
- Setup OpenGL state (viewport, depth test)
- Setup ImGui (docking enabled)
- Load default shader
- Load default mesh(es)
- Load default texture
- Load some OBJ meshes into MeshManager

**Why:** “Proof of pipeline” matters more than perfect architecture early. Once the loop is stable, everything else can be modularized.

**See:** [`MoltenEngine/src/EngineContext.cpp`](MoltenEngine/src/EngineContext.cpp)

### Update (every frame)
- Poll input (camera movement)
- Start ImGui frame
- Draw Editor UI windows (Hierarchy, Inspector, Assets, Camera)
- UI pushes messages (create/delete/import/assign)
- Engine pops and executes messages (mutating Scene + Managers)

**Why:** UI stays lightweight and doesn’t own core rules. It can be rewritten later without breaking the engine.

**See:**
- UI logic: [`MoltenEngine/src/ui/UIManager.cpp`](MoltenEngine/src/ui/UIManager.cpp)
- Message queue: [`MoltenEngine/src/message/MessageQueue.hpp`](MoltenEngine/src/message/MessageQueue.hpp)
- Scene mutations: [`MoltenEngine/src/Scene.hpp`](MoltenEngine/src/Scene.hpp)

### Render (every frame)
- Clear buffers
- Render all SceneObjects
- Render ImGui on top
- Swap buffers

**See:** [`MoltenEngine/src/Renderer.cpp`](MoltenEngine/src/Renderer.cpp)

---

## 3. Project structure map

This repo is split into “engine core”, “editor UI”, “assets/managers”, and “messages”.

### Assets
- `assets/models/*.obj`
- `assets/textures/*.png`
- `progress/*.gif` (great for README / demo)

### External dependencies
- `external/glfw`, `external/glad`, `external/glm`, `external/imgui`, `external/stb`

### Engine overview [links[↗]]
| Area                     | Responsibility                                                                  | Key files                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| ------------------------ | ------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Engine orchestration** | Init / update / render / shutdown flow + message execution                      | [`MoltenEngine/src/EngineContext.cpp`](MoltenEngine/src/EngineContext.cpp)<br>[`MoltenEngine/src/EngineContext.hpp`](MoltenEngine/src/EngineContext.hpp)<br>[`MoltenEngine/src/main.cpp`](MoltenEngine/src/main.cpp)                                                                                                                                                                                                                                                                                                                                                                                 |
| **Scene model**          | Stores entity objects + selection-safe deletion + transform/component ownership | [`MoltenEngine/src/Scene.cpp`](MoltenEngine/src/Scene.cpp)<br>[`MoltenEngine/src/Scene.hpp`](MoltenEngine/src/Scene.hpp)<br><br>[`MoltenEngine/src/Entity.cpp`](MoltenEngine/src/Entity.cpp)<br>[`MoltenEngine/src/Entity.hpp`](MoltenEngine/src/Entity.hpp)<br><br>[`MoltenEngine/src/Transform.cpp`](MoltenEngine/src/Transform.cpp)<br>[`MoltenEngine/src/Transform.hpp`](MoltenEngine/src/Transform.hpp)<br><br>[`MoltenEngine/src/MeshComponent.cpp`](MoltenEngine/src/MeshComponent.cpp)<br>[`MoltenEngine/src/MeshComponent.hpp`](MoltenEngine/src/MeshComponent.hpp)                         |
| **Rendering**            | Draw pipeline (MVP + textures) + render submission/draw traversal               | [`MoltenEngine/src/Renderer.cpp`](MoltenEngine/src/Renderer.cpp)<br>[`MoltenEngine/src/Renderer.hpp`](MoltenEngine/src/Renderer.hpp)<br><br>[`MoltenEngine/src/Shader.cpp`](MoltenEngine/src/Shader.cpp)<br>[`MoltenEngine/src/Shader.hpp`](MoltenEngine/src/Shader.hpp)<br><br>[`MoltenEngine/src/ShaderManager.cpp`](MoltenEngine/src/ShaderManager.cpp)<br>[`MoltenEngine/src/ShaderManager.hpp`](MoltenEngine/src/ShaderManager.hpp)<br><br>[`MoltenEngine/src/ShaderSource.cpp`](MoltenEngine/src/ShaderSource.cpp)<br>[`MoltenEngine/src/ShaderSource.hpp`](MoltenEngine/src/ShaderSource.hpp) |
| **Assets**               | GPU upload + asset file import/parsing                                          | [`MoltenEngine/src/Mesh.cpp`](MoltenEngine/src/Mesh.cpp)<br>[`MoltenEngine/src/Mesh.hpp`](MoltenEngine/src/Mesh.hpp)<br><br>[`MoltenEngine/src/Texture.cpp`](MoltenEngine/src/Texture.cpp)<br>[`MoltenEngine/src/Texture.hpp`](MoltenEngine/src/Texture.hpp)<br><br>[`MoltenEngine/src/ObjLoader.cpp`](MoltenEngine/src/ObjLoader.cpp)<br>[`MoltenEngine/src/ObjLoader.hpp`](MoltenEngine/src/ObjLoader.hpp)                                                                                                                                                                                         |
| **Managers**             | Caching + ownership (load once, reuse handles, manage lifetime)                 | [`MoltenEngine/src/MeshManager.cpp`](MoltenEngine/src/MeshManager.cpp)<br>[`MoltenEngine/src/MeshManager.hpp`](MoltenEngine/src/MeshManager.hpp)<br><br>[`MoltenEngine/src/TextureManager.cpp`](MoltenEngine/src/TextureManager.cpp)<br>[`MoltenEngine/src/TextureManager.hpp`](MoltenEngine/src/TextureManager.hpp)                                                                                                                                                                                                                                                                                 |
| **Editor**               | UI windows + widgets + theme (ImGui layer)                                      | [`MoltenEngine/src/ui/UIManager.cpp`](MoltenEngine/src/ui/UIManager.cpp)<br>[`MoltenEngine/src/ui/UIManager.hpp`](MoltenEngine/src/ui/UIManager.hpp)<br><br>[`MoltenEngine/src/ui/EditorStyle.cpp`](MoltenEngine/src/ui/EditorStyle.cpp)<br>[`MoltenEngine/src/ui/EditorStyle.hpp`](MoltenEngine/src/ui/EditorStyle.hpp)<br><br>[`MoltenEngine/src/ui/EditorWidgets.cpp`](MoltenEngine/src/ui/EditorWidgets.cpp)<br>[`MoltenEngine/src/ui/EditorWidgets.hpp`](MoltenEngine/src/ui/EditorWidgets.hpp)                                                                                                 |
| **Messages**             | Commands from UI → engine (decouple UI actions from core logic)                 | [`MoltenEngine/src/message/Message.hpp`](MoltenEngine/src/message/Message.hpp)<br>[`MoltenEngine/src/message/MessageQueue.hpp`](MoltenEngine/src/message/MessageQueue.hpp)<br><br>Concrete messages: `CreateEntityMessage`, `DeleteEntityMessage`, `ImportMeshMessage`, `ImportTextureMessage`, `SetEntityMeshMessage`, `SetEntityTextureMessage`                                                                                                                                                                                                                                                    |


---

## 4. The data model: Scene → Objects → Components

MoltenEngine uses a simple ECS-ish model (not a full ECS):

### SceneObject (what one “entity” holds)
Each object stores:
- `Entity entity` (ID)
- `Transform transform` (position/rotation/scale)
- `MeshComponent mesh` (Mesh pointer)
- `std::string name`
- `meshKey` + `textureKey` for UI + drag/drop lookup
- `Texture* texture` for per-entity binding

**Why the keys exist (yes it’s hacky but useful):**
- UI needs something stable to display and drag-drop
- Managers store assets by string key
- Storing only pointers would make the inspector harder (you can’t label a pointer meaningfully)
- This helps debug “why is it drawing wrong?” quickly

**Your current struct (source of truth):**  
[`MoltenEngine/src/Scene.hpp`](MoltenEngine/src/Scene.hpp)

### Scene storage choice
Scene stores objects in a `std::vector<SceneObject>`.

**Why a vector:**
- Simple iteration (great for renderer)
- Simple inspector selection by index
- Fast enough for a small engine prototype

**Trade-off:**
Deleting an object shifts indices, so selection must be fixed.

### Deletion strategy (the important part)
MoltenEngine supports:
- `DestroyObjectAt(index)` for UI convenience
- `DestroyObject(entity)` for future-proofing
- `DeleteEntity(entity, selectedIndex)` as the improved version that also repairs selection

**Why the improved delete exists:**
- Prevents `selectedIndex` from pointing into freed/shifted objects
- Avoids “selection points into garbage” bugs (classic editor crash)

**See:** `Scene::DeleteEntity()` in [`MoltenEngine/src/Scene.hpp`](MoltenEngine/src/Scene.hpp)

---

## 5. Rendering pipeline (MVP + textures)

Rendering happens in `Renderer::RenderScene(scene, camera)`.

### Per-object rendering steps
For each SceneObject:
1. Compute Model matrix from `Transform`
2. Compute View and Projection from `Camera`
3. Build MVP = projection * view * model
4. Bind shader
5. Set MVP uniform
6. Choose texture (per-entity or default)
7. Bind mesh VAO and draw indexed triangles

**Why it’s structured this way:**
- Renderer stays responsible for GPU calls
- Scene stays purely data
- Per-entity textures are supported naturally
- “Black model” issues are prevented by a default texture fallback

**See:** [`MoltenEngine/src/Renderer.cpp`](MoltenEngine/src/Renderer.cpp)  
Shader inputs: [`MoltenEngine/src/ShaderSource.cpp`](MoltenEngine/src/ShaderSource.cpp)

---

## 6. Asset pipeline (Mesh + Texture + OBJ)

### Mesh (GPU buffer owner)
Mesh owns VAO/VBO/EBO and knows its index count.

**Why Mesh encapsulates OpenGL buffers:**
- Keeps OpenGL boilerplate out of EngineContext/Renderer
- Enforces RAII-ish cleanup in destructor
- Makes draw calls clean (`mesh->Bind()`)

**See:** [`MoltenEngine/src/Mesh.cpp`](MoltenEngine/src/Mesh.cpp)

### MeshManager (asset cache)
MeshManager owns meshes using `std::unique_ptr<Mesh>`.

**Why:**
- Prevents memory leaks
- Lets multiple entities share the same mesh pointer safely
- Makes “import mesh” behave like an engine system, not a special case

**See:** [`MoltenEngine/src/MeshManager.cpp`](MoltenEngine/src/MeshManager.cpp)

### Texture + TextureManager (same idea)
Texture loads image data via stb_image, uploads to OpenGL, and binds later.
TextureManager caches textures by key.

**Why:**
- Supports per-entity textures
- Supports drag/drop asset assignment by key
- Lets renderer pick defaults cleanly

**See:**
- [`MoltenEngine/src/Texture.cpp`](MoltenEngine/src/Texture.cpp)
- [`MoltenEngine/src/TextureManager.cpp`](MoltenEngine/src/TextureManager.cpp)

### OBJ Loader (file → vertex/index arrays)
OBJ loader outputs `ObjMeshData { vertices, indices }` in the format MoltenEngine expects.

**MoltenEngine vertex format:**
- Position (x, y, z)
- UV (u, v)

So each vertex is **5 floats**.

**Why:** It matches the shader input (`aPos` + `aUV`) and the Mesh stride.

**See:**
- [`MoltenEngine/src/ObjLoader.cpp`](MoltenEngine/src/ObjLoader.cpp)
- [`MoltenEngine/src/ShaderSource.cpp`](MoltenEngine/src/ShaderSource.cpp)

---

## 7. Editor UI (Hierarchy / Inspector / Assets)

The editor is built with ImGui and is split into:
- `EditorStyle` (theme)
- `EditorWidgets` (custom molten slider, styling helpers)
- `UIManager` (windows: hierarchy/inspector/assets/camera)

### Hierarchy window
- Shows list of objects by name
- Selection changes `selectedIndex`
- Create buttons add new entities
- Delete button removes selected entity

**See:** [`MoltenEngine/src/ui/UIManager.cpp`](MoltenEngine/src/ui/UIManager.cpp)

### Inspector window
Edits selected object:
- Name
- Mesh/model selection (combo + drag/drop)
- Texture selection (combo + drag/drop)
- Transform editing (DragFloat3)

**Why combo + drag/drop:**
- Combo proves “systems exist”
- Drag/drop proves “editor workflow”

### Assets window
- Import OBJ: path + key
- Import texture: path + key
- Lists loaded meshes/textures
- Supports drag payloads

---

## 8. Message system (UI asks, Engine does)

This is the architectural boundary that prevents UI spaghetti.

### MessageQueue
- UI pushes `unique_ptr<Message>`
- Engine drains queue and executes actions

**Why messages:**
- UI doesn’t need to know engine internals
- Engine owns mutation rules (safe deletes, default textures, manager caching)
- Makes it easier later to add undo/redo, logging, replay, or networking

**See:**
- [`MoltenEngine/src/message/Message.hpp`](MoltenEngine/src/message/Message.hpp)
- [`MoltenEngine/src/message/MessageQueue.hpp`](MoltenEngine/src/message/MessageQueue.hpp)

---

## 9. Lifetime + shutdown (avoiding leaks and GL traps)

OpenGL resources should be deleted **before the OpenGL context is destroyed**.

### Shutdown order
1. Shutdown ImGui backends
2. Clear asset managers (delete textures and meshes while context is valid)
3. Destroy window, terminate GLFW

**Why:** If you delete OpenGL objects after the context is gone, behavior becomes undefined (and drivers vary).

**Enforced in:** `EngineContext::Terminate()`  
**See:** [`MoltenEngine/src/EngineContext.cpp`](MoltenEngine/src/EngineContext.cpp)

---

## 10. Where to showcase code snippets proudly

You can paste *small*, intentional code excerpts below.  
Keep each snippet 10–25 lines max so it looks deliberate.

### Showcase A — MVP pipeline in Renderer
**File:** [`MoltenEngine/src/Renderer.cpp`](MoltenEngine/src/Renderer.cpp)

```cpp
// PASTE HERE: The section that computes model/view/projection and uploads MVP.
