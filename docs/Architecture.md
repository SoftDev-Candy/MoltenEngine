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
| 12 | [2026 upgrade snapshot](#12-2026-upgrade-snapshot) | Gameplay, gizmos, lighting, and editor upgrades |

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
- [`../src/main.cpp`](../src/main.cpp)
- [`../src/EngineContext.cpp`](../src/EngineContext.cpp) / [`../src/EngineContext.hpp`](../src/EngineContext.hpp)

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

**See:** [`../src/EngineContext.cpp`](../src/EngineContext.cpp)

### Update (every frame)
- Poll input (camera movement / play controls)
- Start ImGui frame
- Draw either editor UI or play HUD depending on engine mode
- UI pushes messages (create/delete/import/assign/play/stop)
- Engine pops and executes messages (mutating Scene + Managers)
- Run either editor camera controls or `SplineShooterGame`

**Why:** UI stays lightweight and doesn’t own core rules. It can be rewritten later without breaking the engine.

### Mode split (editor vs play)
- **Editor mode** draws the docked editor windows, gizmos, lights panel, and asset browser
- **Play mode** hides editor panels and only draws the game HUD / main menu overlay
- Messages are flushed before the main editor/game logic so `Play` / `Stop` applies immediately that frame

**See:**
- UI logic: [`../src/ui/UIManager.cpp`](../src/ui/UIManager.cpp)
- Message queue: [`../src/message/MessageQueue.hpp`](../src/message/MessageQueue.hpp)
- Scene mutations: [`../src/Scene.hpp`](../src/Scene.hpp)

### Render (every frame)
- Clear buffers
- Render all SceneObjects
- Render ImGui on top
- Swap buffers

**See:** [`../src/Renderer.cpp`](../src/Renderer.cpp)

---

## 3. Project structure map

This repo is split into “engine core”, “editor UI”, “assets/managers”, and “messages”.

### Assets
- `assets/models/*.obj`
- `assets/textures/*.png`
- `progress/*.gif` (great for README / demo)

### External dependencies
- `external/glfw`, `external/glad`, `external/glm`, `external/imgui`, `external/ImGuizmo`, `external/stb`

### Engine overview [links[↗]]
| Area                     | Responsibility                                                                  | Key files                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                            |
| ------------------------ | ------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| **Engine orchestration** | Init / update / render / shutdown flow + message execution                      | [`../src/EngineContext.cpp`](../src/EngineContext.cpp)<br>[`../src/EngineContext.hpp`](../src/EngineContext.hpp)<br>[`../src/main.cpp`](../src/main.cpp)                                                                                                                                                                                                                                                                                                                                                                                 |
| **Scene model**          | Stores entity objects + selection-safe deletion + transform/component ownership | [`../src/Scene.cpp`](../src/Scene.cpp)<br>[`../src/Scene.hpp`](../src/Scene.hpp)<br><br>[`../src/Entity.cpp`](../src/Entity.cpp)<br>[`../src/Entity.hpp`](../src/Entity.hpp)<br><br>[`../src/Transform.cpp`](../src/Transform.cpp)<br>[`../src/Transform.hpp`](../src/Transform.hpp)<br><br>[`../src/MeshComponent.cpp`](../src/MeshComponent.cpp)<br>[`../src/MeshComponent.hpp`](../src/MeshComponent.hpp)                         |
| **Rendering**            | Draw pipeline (MVP + textures) + render submission/draw traversal               | [`../src/Renderer.cpp`](../src/Renderer.cpp)<br>[`../src/Renderer.hpp`](../src/Renderer.hpp)<br><br>[`../src/Shader.cpp`](../src/Shader.cpp)<br>[`../src/Shader.hpp`](../src/Shader.hpp)<br><br>[`../src/ShaderManager.cpp`](../src/ShaderManager.cpp)<br>[`../src/ShaderManager.hpp`](../src/ShaderManager.hpp)<br><br>[`../src/ShaderSource.cpp`](../src/ShaderSource.cpp)<br>[`../src/ShaderSource.hpp`](../src/ShaderSource.hpp) |
| **Assets**               | GPU upload + asset file import/parsing                                          | [`../src/Mesh.cpp`](../src/Mesh.cpp)<br>[`../src/Mesh.hpp`](../src/Mesh.hpp)<br><br>[`../src/Texture.cpp`](../src/Texture.cpp)<br>[`../src/Texture.hpp`](../src/Texture.hpp)<br><br>[`../src/ObjLoader.cpp`](../src/ObjLoader.cpp)<br>[`../src/ObjLoader.hpp`](../src/ObjLoader.hpp)                                                                                                                                                                                         |
| **Managers**             | Caching + ownership (load once, reuse handles, manage lifetime)                 | [`../src/MeshManager.cpp`](../src/MeshManager.cpp)<br>[`../src/MeshManager.hpp`](../src/MeshManager.hpp)<br><br>[`../src/TextureManager.cpp`](../src/TextureManager.cpp)<br>[`../src/TextureManager.hpp`](../src/TextureManager.hpp)                                                                                                                                                                                                                                                                                 |
| **Editor**               | UI windows + widgets + theme (ImGui layer)                                      | [`../src/ui/UIManager.cpp`](../src/ui/UIManager.cpp)<br>[`../src/ui/UIManager.hpp`](../src/ui/UIManager.hpp)<br><br>[`../src/ui/EditorStyle.cpp`](../src/ui/EditorStyle.cpp)<br>[`../src/ui/EditorStyle.hpp`](../src/ui/EditorStyle.hpp)<br><br>[`../src/ui/EditorWidgets.cpp`](../src/ui/EditorWidgets.cpp)<br>[`../src/ui/EditorWidgets.hpp`](../src/ui/EditorWidgets.hpp)                                                                                                 |
| **Messages**             | Commands from UI → engine (decouple UI actions from core logic)                 | [`../src/message/Message.hpp`](../src/message/Message.hpp)<br>[`../src/message/MessageQueue.hpp`](../src/message/MessageQueue.hpp)<br><br>Concrete messages: `CreateEntityMessage`, `DeleteEntityMessage`, `ImportMeshMessage`, `ImportTextureMessage`, `SetEntityMeshMessage`, `SetEntityTextureMessage`                                                                                                                                                                                                                                                    |
| **Gameplay**             | Spline shooter runtime logic + play HUD + round flow                            | [`../game/Game.cpp`](../game/Game.cpp)<br>[`../game/Game.hpp`](../game/Game.hpp)<br><br>[`../game/GameUI.cpp`](../game/GameUI.cpp)<br>[`../game/GameUI.hpp`](../game/GameUI.hpp)                                                                                                                                                                                                                                                                                                                                 |


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
[`../src/Scene.hpp`](../src/Scene.hpp)

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

**See:** `Scene::DeleteEntity()` in [`../src/Scene.hpp`](../src/Scene.hpp)

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

**See:** [`../src/Renderer.cpp`](../src/Renderer.cpp)  
Shader inputs: [`../src/ShaderSource.cpp`](../src/ShaderSource.cpp)

---

## 6. Asset pipeline (Mesh + Texture + OBJ)

### Mesh (GPU buffer owner)
Mesh owns VAO/VBO/EBO and knows its index count.

**Why Mesh encapsulates OpenGL buffers:**
- Keeps OpenGL boilerplate out of EngineContext/Renderer
- Enforces RAII-ish cleanup in destructor
- Makes draw calls clean (`mesh->Bind()`)

**See:** [`../src/Mesh.cpp`](../src/Mesh.cpp)

### MeshManager (asset cache)
MeshManager owns meshes using `std::unique_ptr<Mesh>`.

**Why:**
- Prevents memory leaks
- Lets multiple entities share the same mesh pointer safely
- Makes “import mesh” behave like an engine system, not a special case

**See:** [`../src/MeshManager.cpp`](../src/MeshManager.cpp)

### Texture + TextureManager (same idea)
Texture loads image data via stb_image, uploads to OpenGL, and binds later.
TextureManager caches textures by key.

**Why:**
- Supports per-entity textures
- Supports drag/drop asset assignment by key
- Lets renderer pick defaults cleanly

**See:**
- [`../src/Texture.cpp`](../src/Texture.cpp)
- [`../src/TextureManager.cpp`](../src/TextureManager.cpp)

### OBJ Loader (file → vertex/index arrays)
OBJ loader outputs `ObjMeshData { vertices, indices }` in the format MoltenEngine expects.

**MoltenEngine vertex format:**
- Position (x, y, z)
- UV (u, v)

So each vertex is **5 floats**.

**Why:** It matches the shader input (`aPos` + `aUV`) and the Mesh stride.

**See:**
- [`../src/ObjLoader.cpp`](../src/ObjLoader.cpp)
- [`../src/ShaderSource.cpp`](../src/ShaderSource.cpp)

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

**See:** [`../src/ui/UIManager.cpp`](../src/ui/UIManager.cpp)

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

### New editor tooling
- **ImGuizmo** is used for translate / rotate / scale in the main scene window
- **Lights window** can add/delete lights, edit spotlight values, and apply a spline-shooter light preset
- **Scene Hierarchy** can create cubes, empty objects, and auto-numbered `SplinePoint_0..N`
- **Asset browser** now has loaded assets + project browser tabs, source paths, search, quick import, and drag/drop assignment
- **Empty objects** render with editor helper shapes so spline points and placeholders are visible in the scene

### Play HUD separation
- Editor UI is not drawn in Play mode
- Play mode uses `UIManager::DrawPlayHUD(...)`, which delegates the actual layout to `game/GameUI.cpp`
- The play overlay shows a small in-game HUD while playing and a centered menu card for start / win / lose states

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
- [`../src/message/Message.hpp`](../src/message/Message.hpp)
- [`../src/message/MessageQueue.hpp`](../src/message/MessageQueue.hpp)

---

## 9. Lifetime + shutdown (avoiding leaks and GL traps)

OpenGL resources should be deleted **before the OpenGL context is destroyed**.

### Shutdown order
1. Shutdown ImGui backends
2. Clear asset managers (delete textures and meshes while context is valid)
3. Destroy window, terminate GLFW

**Why:** If you delete OpenGL objects after the context is gone, behavior becomes undefined (and drivers vary).

**Enforced in:** `EngineContext::Terminate()`  
**See:** [`../src/EngineContext.cpp`](../src/EngineContext.cpp)

---

## 12. 2026 upgrade snapshot

This section is the “what the engine actually does now” cheat sheet.

### Gameplay layer
- `SplineShooterGame` is the current game-side runtime layer
- Supports `Start`, `Playing`, `Win`, and `Lose`
- Win triggers when the player reaches the end of the spline
- Player movement supports both spline-follow and free-fly modes
- Runtime-only bullets, obstacles, and backdrop asteroids are spawned and cleaned without polluting saved scenes

### Spline path rules
- Control points are collected from scene objects named `SplinePoint_0 .. SplinePoint_n`
- Points are sorted by numeric suffix before Catmull-Rom sampling
- Player follows the spline centerline, then applies lane offsets for dodge movement
- Camera now uses a separate rail-follow rig so player strafing does not yank the whole camera sideways

### Asteroid variants
- Two asteroid meshes are supported in gameplay: `Asteroid_1d` and `Asteroid_1c`
- Primary asteroid uses `Asteroid1d_Color_1K`
- Secondary asteroid uses `9SliceTableTexture` + `9SliceTableTexture_Specular`
- Current shader does not consume a normal map yet, so `9SliceTableTexture_Normal` is imported for future use but not rendered as a normal-mapped material
- Primary asteroid hits harder (double contact damage), secondary asteroid is the lighter 1-damage variant

### Space feel and lighting
- Play mode uses a follow light that sits behind and above the player, points forward, and restores the editor light state on stop
- Runtime backdrop asteroids are spawned around the spline so the player reads as flying through an asteroid field instead of empty space
- Clear color was pushed darker to feel more space-like

### Asset and scene stability
- Scene serialization remains key-based (`meshKey`, `textureKey`, `albedoKey`, `specularKey`)
- Raw mesh / texture pointers are rebound after load through the managers
- Auto-import helpers now pick up gameplay assets like asteroid meshes and play textures when those files exist in the project assets folder

---

## Code Snippets

### Showcase A — MVP pipeline in Renderer
**File:** [`../src/Renderer.cpp`](../src/Renderer.cpp)

```
for (auto& obj:scene.GetObjects())
    {
        if (obj.mesh.mesh ==nullptr)
        {
            continue;
        }
        else
        {
            //1.Compute Model
            glm::mat4 model = obj.transform.GetMatrix();

            //2.Compute MVP
            glm::mat4 mvp = projection * view * model;
            //bind shader with MVP
            if (shaderptr == nullptr)
            {
                std::cout<<"The activeShader is null ERROR in File Renderer.cpp (RenderScene)\n";
                continue;
            }
           else {
                shaderptr->bind();
               shaderptr->setMat4("uModel", model);
                shaderptr->setMat4("MVP" ,mvp);
               shaderptr->setMat4("uLightSpaceMatrix", lightSpace);
               shaderptr->setInt("uShadowMap", 2);
               shaderptr->setInt("uShadowsEnabled", shadowsEnabled ? 1 : 0);

               // bind shadow depth texture to unit 2
               glActiveTexture(GL_TEXTURE2);
               glBindTexture(GL_TEXTURE_2D, shadowDepthTex);

               auto& lights = scene.GetLights();

               int count = (int)lights.size();
               if (count > 8) count = 8;

               shaderptr->setInt("uLightCount", count);

               for (int i = 0; i < count; ++i)
               {
                   const auto& L = lights[i];

                   shaderptr->setVec3(std::string("uLightPos[") + std::to_string(i) + "]", L.position);
                   shaderptr->setVec3(std::string("uLightColor[") + std::to_string(i) + "]", L.color);
                   shaderptr->setFloat(std::string("uLightIntensity[") + std::to_string(i) + "]", L.intensity);
                   shaderptr->setFloat(std::string("uLightAmbient[") + std::to_string(i) + "]", L.ambientStrength);
                   // rotation is degrees -> convert
                   glm::vec3 rotRad = glm::radians(L.rotation);

                   glm::mat4 R(1.0f);
                   R = glm::rotate(R, rotRad.x, glm::vec3(1,0,0));
                   R = glm::rotate(R, rotRad.y, glm::vec3(0,1,0));
                   R = glm::rotate(R, rotRad.z, glm::vec3(0,0,1));

                   // OpenGL “forward” is typically -Z
                   glm::vec3 dir = glm::normalize(glm::vec3(R * glm::vec4(0,0,-1,0)));

                   shaderptr->setVec3("uLightDir[" + std::to_string(i) + "]", dir);


                   // angles are degrees in UI -> cos expects radians
                   float innerCos = cosf(glm::radians(L.innerAngle));
                   float outerCos = cosf(glm::radians(L.outerAngle));

                   shaderptr->setFloat(std::string("uLightInnerCos[") + std::to_string(i) + "]", innerCos);
                   shaderptr->setFloat(std::string("uLightOuterCos[") + std::to_string(i) + "]", outerCos);


               }

               // camera position (public in Camera class)
               shaderptr->setVec3("uViewPos", cam.position);

    shaderptr->setFloat("uShininess", obj.shininess);
               shaderptr->setFloat("uSpecStrength", 1.0);

            }
            // --- Material binding (Phase 2A core) ---
            Texture* albedoTex = nullptr;
            Texture* specTex   = nullptr;

            // Albedo priority: per-entity albedo -> old per-entity texture -> engine active -> default
            if (obj.albedo) albedoTex = obj.albedo;
            else if (obj.texture) albedoTex = obj.texture;
            else if (textureptr) albedoTex = textureptr;
            else albedoTex = defaultTexture;

            // Specular: per-entity specular -> fallback to albedo (better than null)
            if (obj.specular) specTex = obj.specular;
            else specTex = albedoTex;

            // Bind albedo to unit 0
            glActiveTexture(GL_TEXTURE0);
            if (albedoTex) albedoTex->Bind();

            // Bind specular to unit 1
            glActiveTexture(GL_TEXTURE1);
            if (specTex) specTex->Bind();

            // Tell shader which units to read
            shaderptr->setInt("uAlbedoMap", 0);
            shaderptr->setInt("uSpecularMap", 1);


            obj.mesh.mesh->Bind();
            glDrawElements(GL_TRIANGLES, obj.mesh.mesh->GetIndexCount(), GL_UNSIGNED_INT, 0);

        }
    }
```

## Showcase B — MeshManager caching (engine-quality)

**File:** [`../src/MeshManager.cpp`](../src/MeshManager.cpp)

```
Mesh * MeshManager::Add(const std::string key, std::unique_ptr<Mesh> mesh)
{
    //1. To be safe lets not overwrite in mem just find it and if it exists return//
    auto it = meshes.find(key);
    if (it != meshes.end())
    {
        return it->second.get();
    }
//2. Store it

    meshes.emplace(key,std::move(mesh));

//3.Return the RAWWWWWWR pointer to the stored mesh//
    return meshes.at(key).get();

}
```

## Showcase C — MessageQueue swap trick (clean design)

File: ../src/message/MessageQueue.hpp

```
   std::vector<std::unique_ptr<Message>> PopAll()
    {
        std::lock_guard<std::mutex> lock(mutex);

        std::vector<std::unique_ptr<Message>> out;
        out.swap(messages); // fast + leaves messages empty

        return out;
    }
```

## Showcase D — Drag/drop payload (editor workflow)

File: ../src/ui/UIManager.cpp

```
ImGui::TextUnformatted("Textures (drag onto Texture)");
    {
        std::vector<std::string> keys = listTextureKeys();
        for (const std::string& k : keys)
        {
            ImGui::Selectable(k.c_str());

            //Drag source//
            if (ImGui::BeginDragDropSource())
            {
                ImGui::SetDragDropPayload("TEX_KEY", k.c_str(), k.size() + 1);
                ImGui::Text("Texture: %s", k.c_str());
                ImGui::EndDragDropSource();
            }
        }
    }

```
**Payload delivery**
```
                //Drag target for meshes//
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("MESH_KEY"))
                    {
                        const char* droppedKey = (const char*)payload->Data;
                        Mesh* m = getMeshByKey(droppedKey);
                        if (m != nullptr)
                        {
                            obj.mesh.mesh = m;
                            obj.meshKey = droppedKey;
                        }
                    }
                    ImGui::EndDragDropTarget();
                }
            }

```
## Showcase E — OBJ import pipeline (file → GPU mesh)

File: ../src/EngineContext.cpp

```
bool EngineContext::ImportObjAsMesh(const std::string& key, const std::string& path)
{
    ObjMeshData imported = LoadOBJ(path, false);

    if (imported.vertices.empty() || imported.indices.empty())
    {
        std::cerr << "[EngineContext] ImportObjAsMesh failed: empty mesh data\n";
        return false;
    }

    //FIXME: Mesh expects 5 floats per vertex (pos3 + uv2)//
    if ((imported.vertices.size() % 8) != 0)
    {
        std::cerr << "[EngineContext] WARNING: OBJ vertices not multiple of 8 (pos+uv+normal).\n";
    }

    meshmanager.Add(key,
        std::make_unique<Mesh>(
            imported.vertices.data(),
            imported.vertices.size(),
            imported.indices.data(),
            imported.indices.size() , 8
        )
    );

    return true;
}
```
## 11. Known limitations + next steps

# Known limitations
* Scene Storage: Scene uses vector storage (deletions shift indices; selection is repaired but long-term ECS would help).
* OBJ Loader: OBJ loader targets a simple vertex format (pos+uv); normals/materials are not fully used.
* Shaders: One shader for everything (no real material system yet).
* Undo/Redo: No undo/redo (messages make this possible later).
*File loading and persistence management Missing

# Next steps (logical upgrades)
* Material System: Add a Material struct (texture + shader parameters).
* Command History: Add undo/redo by storing reverse operations in the message system.
* Serialization: Add scene serialization (save/load entities and assignments).
* Camera: Upgrade camera to support rotation (yaw/pitch) and expose it in the UI.
