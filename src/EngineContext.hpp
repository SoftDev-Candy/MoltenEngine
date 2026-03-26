//
// Created by Candy on 11/21/2025.
//

#ifndef B_WENGINE_ENGINECONTEXT_HPP
#define B_WENGINE_ENGINECONTEXT_HPP
#include<iostream>
#include<glad/glad.h>
#include <memory>
#include<GLFW/glfw3.h>
#include "Renderer.hpp"
#include "Scene.hpp"
#include "Camera.hpp"
#include "MeshManager.hpp"
#include "Shader.hpp"
#include "ShaderManager.hpp"
#include "ui/UIManager.hpp"
#include "Texture.hpp"
#include "TextureManager.hpp"
#include "message/MessageQueue.hpp"
#include "message/Message.hpp"
#include "Game.hpp"

enum class EngineMode
{
    Editor , Play
};

class EngineContext
{

public:
//Creating Camera object
Camera camera;
    Entity CreateCube(const std::string& cubename);
    Entity CreateEntityWithMesh(const std::string& name, const std::string& meshKey);

    void init();
    bool ShouldClose();
    void update();
    void Render();
    void Terminate();
    void AddObject();

    std::vector<SceneObject>& Getobject() ;

    //Texture helper Functions
    bool ImportObjAsMesh(const std::string& key, const std::string& path);
    bool ImportTexture(const std::string& key, const std::string& path);

    //Mesh and Texture objs
    Mesh* cubeMesh;
    Texture* texture;

    //Respective Manager Objects
    TextureManager textureManager;
    ShaderManager shadermanager;
    MeshManager meshmanager;
    UIManager ui;
    MessageQueue messagequeue;


    //Delete Function
    bool DeleteSelectedObject();
    bool DeleteEntity(Entity e);


    void PushMessage(std::unique_ptr<Message> msg);
    void ProcessMessages();
    void SetEntityMesh(Entity e, const std::string& meshKey);
    void SetEntityTexture(Entity e, const std::string& textureKey);


    Scene& GetScene()
    {
        return scene;
    }
    const Scene& GetScene() const
    {
        return scene;
    }

    void SetMipmapsEnabled(bool enabled);
    bool GetMipmapsEnabled() const
    {
        return useMipmaps;
    }

    void SetEntityAlbedo(Entity e, const std::string& albedoKey);
    void SetEntitySpecular(Entity e, const std::string& specularKey);
    void SetEntityShininess(Entity e, float shininess);
    void SetShadowsEnabled(bool enabled);

    //Camera Function referencing the camera class
    void CameraControls (Camera& camera);

    //Calculate FPS
    void DisplayCalculateFrameRate();
    SceneObject* FindObject(Entity e);

    //Save and Load Scene functions
    bool SaveScene(const std::string& path);
    bool LoadScene(const std::string& path);
    void ResolveSceneAfterLoad();
    void RebindSceneAssetPointers();

    //Mipmaps Bool variables public for now
    bool mipmapsEnabled_ = true;
    bool shadowsEnabled_ = true;

    //Game Start and Stop Functions
    void StartGame();
    void StopGame();
    void UpdatePlayFollowLight();

    //Engine Mode enum class call
    EngineMode mode_ = EngineMode::Editor;

    //Call the Shooter Game Class add a little '_' to show DO NOT TOUCH THIS PLEASE//
    SplineShooterGame splineGame_;

private:

    GLFWwindow* window;
    Scene scene;
    Renderer renderer;
    int selectedIndex = -1;
    bool useMipmaps = true;
    float deltaTime = 0.0f;
    double lastTime = 0.0f;

    //To calculate FPS time//
    double fpsTimer = 0.0f;
    int fpsFrames = 0;
    float fpsValue = 0.0f;
    float msValue = 0.0f;

    //For mouse camera movement
    bool mouseLookActive = false;
    bool firstMouse = true;
    double lastMouseX = 0.0, lastMouseY = 0.0;
    bool toggleControlModeKeyWasDown_ = false;
    bool startRoundKeyWasDown_ = false;
    bool shootKeyWasDown_ = false;
    bool stopGameKeyWasDown_ = false;
    int playFollowLightIndex_ = -1;
    bool playFollowLightAdded_ = false;
    bool playFollowLightHadOriginal_ = false;
    Light cachedPlayFollowLight_{};

    float mouseSensitivity = 0.12f;   // tweak
    float pitchClamp = 89.0f;


};

#endif //B_WENGINE_ENGINECONTEXT_HPP
