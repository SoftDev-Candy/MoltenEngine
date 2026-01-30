//
// Created by Candy on 11/21/2025.
//

#include "EngineContext.hpp"
#include "ObjLoader.hpp"
#include "ShaderSource.hpp"
#include"../external/imgui/imgui.h"
#include"../external/imgui/imgui_impl_glfw.h"
#include"../external/imgui/imgui_impl_opengl3.h"
#include "message/CreateEntityMessage.hpp"
#include "message/DeleteEntityMessage.hpp"
#include "message/FileDroppedMessage.hpp"
#include "message/ImportMeshMessage.hpp"
#include "message/ImportTextureMessage.hpp"
#include "message/SetEntityMeshMessage.hpp"
#include "message/SetEntityTextureMessage.hpp"
#include "ui/EditorStyle.hpp"
#include "ui/EditorWidgets.hpp"
#include "MemoryDebug.hpp"


bool EngineContext::ImportTexture(const std::string& key, const std::string& path)
{
    Texture* t = textureManager.Add(key, std::make_unique<Texture>(path.c_str()));
    if (t)
    {
        t->ApplySampling(useMipmaps); // <-- IMPORTANT: new texture respects UI toggle
    }
    return t != nullptr;
}

void EngineContext::SetEntityMesh(Entity e, const std::string& meshKey)
{
    Mesh* m = meshmanager.Get(meshKey);
    if (m == nullptr)
        return;

    auto& objects = scene.GetObjects();
    for (auto& obj : objects)
    {
        if (obj.entity.Id == e.Id)
        {
            obj.mesh.mesh = m;
            obj.meshKey = meshKey;
            return;
        }
    }
}

void EngineContext::SetEntityTexture(Entity e, const std::string& textureKey)
{
    Texture* t = textureManager.Get(textureKey);
    if (t == nullptr)
        return;

    auto& objects = scene.GetObjects();
    for (auto& obj : objects)
    {
        if (obj.entity.Id == e.Id)
        {
            obj.texture = t;
            obj.textureKey = textureKey;
            return;
        }
    }
}


    bool EngineContext::DeleteSelectedObject()
    {
        //If nothing selected, do nothing
        if (selectedIndex < 0 || selectedIndex >= (int)scene.GetObjects().size())
        {
            return false;
        }

        //Delete it
        scene.DestroyObjectAt((size_t)selectedIndex);

        //Fix selectedIndex so it doesn't point into garbage
        int count = (int)scene.GetObjects().size();
        if (count == 0)
        {
            selectedIndex = -1;
        }
        else if (selectedIndex >= count)
        {
            selectedIndex = count - 1;
        }

        return true;
    }

void EngineContext::PushMessage(std::unique_ptr<Message> msg)
{
    messagequeue.Push(std::move(msg));

}

void EngineContext::ProcessMessages()
{
    //Grab everything pushed this frame (fast swap)
    auto msgs = messagequeue.PopAll();

    //Process in order
    for (auto& m : msgs)
    {
        if (!m) continue;

        //This assumes your Message base class has Execute(EngineContext&)
        m->Dispatch(*this);
    }

}


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


static void framebuffer_size_callback(GLFWwindow* , int w , int h)
{

    glViewport(0,0,w,h);

}

Entity EngineContext::CreateCube(const std::string &cubename)
{
    scene.CreateObject();
    auto& obj = scene.GetObjects().back();
    obj.mesh.mesh = meshmanager.Get("Cube");
    obj.meshKey = "Cube"; //FIXME: UI needs this for drag/drop

    obj.name = cubename;

    //Default texture per entity (so it NEVER shows up black by accident)//
    obj.texture = textureManager.Get("Default");
    obj.textureKey = "Default";

    return obj.entity;
}


Entity EngineContext::CreateEntityWithMesh(const std::string &name, const std::string &meshKey)
{
    scene.CreateObject();
    auto& obj = scene.GetObjects().back();

    obj.name = name;

    // we are adding this as a safety check so if meshkey is wrong or null. Renderer skip that schizer
    obj.mesh.mesh = meshmanager.Get(meshKey);
    obj.meshKey = meshKey; //FIXME: UI needs this for drag/drop

    //Default texture per entity (same logic as cube)//
    obj.texture = textureManager.Get("Default");
    obj.textureKey = "Default";

    return obj.entity;
}

void EngineContext::init()
{

    //Initialize GLFW

    if(!glfwInit())
    {
        std::cout<<"Failed to Initialize GLFW"<<std::endl;
        return ;
    }

    //give hints on what version you are using
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);

    //Create window here now if not created terminate that mfer
    window = glfwCreateWindow(900,800,"B_WEngine",nullptr,nullptr);
    if (window == NULL )
    {
        std::cout<<"Failed to create GLFW Window"<<std::endl;
        glfwTerminate();
        return ;
    }

    //making window the current context //
    glfwMakeContextCurrent(window);

    glfwSetFramebufferSizeCallback(window ,framebuffer_size_callback);
    //Initialize Glad
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {

        std::cerr<<"Failed to initialize GLAD"<<std::endl;
        return ;

    }

    glViewport(2,4,920,800); //This is the viewport or the drawable size area

    //Depth test don't worry about it until it throws error....uhh then get a panic attack and! worry//
    //from the future - Don't do the second part ⬆️
    glEnable(GL_DEPTH_TEST);

    //THIS IS IT FINALLY THE COSMIC CREATION OF THE MOTHERLAND STARDENBURDENHARDENBART DEAR IMGUI//
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    EditorStyle::Apply();
    ImGuiIO& io = ImGui::GetIO();
    //Enabling Docking here
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //Enabling Controller don't need it saw it sounds cool might keep it for now;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; //Enabling Keyboard

    ImGui_ImplGlfw_InitForOpenGL(window,true);
    ImGui_ImplOpenGL3_Init("#version 330");

// TODO: After mesh starts working need to make a mesh manager for all the default objects //
    float vertices[] =
    {

        -0.5f, -0.5f, -0.5f,0.0f,0.0f,
         0.5f, -0.5f, -0.5f,1.0f,0.0f,
         0.5f, 0.5f, -0.5f,1.0f,1.0f,
        -0.5f, 0.5f, -0.5f,0.0f,1.0f,

        -0.5f, -0.5f, 0.5f,0.0f,0.0f,
        0.5f, -0.5f, 0.5f,1.0f,0.0f,
        0.5f, 0.5f, 0.5f,1.0f,1.0f,
        -0.5f, 0.5f, 0.5f,0.0f,1.0f

    };

    unsigned int indices[] =
    {
        0, 1, 2, 2, 3, 0, // back face
        4, 5, 6, 6, 7, 4, // front face
        0, 4, 7, 7, 3, 0, // left face
        1, 5, 6, 6, 2, 1, // right face
        3, 2, 6, 6, 7, 3, // top face
        0, 1, 5, 5, 4, 0 // bottom face
    };

    //FIXME: this is getting too cryptic need to ask for a better solution for this
    //No one is going to understand this jargon //
    //....yeah pastme ---what was i doing here again bruv??//
    cubeMesh= meshmanager.Add( "Cube",
        std::make_unique<Mesh>(vertices, 40, indices,36)
    );
    //FIXME - NEED A MESH MANAGER TO DELETE THIS //

    shadermanager.LoadShader("Default",VertexShaderSource,FragmentShaderSource);
    renderer.SetActiveShader(shadermanager.GetShader("Default"));

    //TODO: move this into TextureManager later, for now just prove pipeline works//
    texture = textureManager.Add("Default", std::make_unique<Texture>("../assets/texture.png"));
    renderer.SetActiveTexture(texture);
    renderer.SetDefaultTexture(texture); //fallback if entity has no texture (shouldn't happen now)


    ObjMeshData imported = LoadOBJ("../assets/models/Cube2.obj", false);

    if(!imported.vertices.empty() && !imported.indices.empty())
    {
        meshmanager.Add("ImportedOBJ",
                        std::make_unique<Mesh>(
                            imported.vertices.data(),
                            imported.vertices.size(),     // float count
                            imported.indices.data(),
                            imported.indices.size(),8     // index count
                        )
        );
    }

    PrintMemoryStatus();

}

bool EngineContext::ShouldClose()
{
    return glfwWindowShouldClose(window);
}

void EngineContext::update()
{

     glfwPollEvents();
    //Start the frame for imgui
    ImGui_ImplGlfw_NewFrame();
    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();

    UI::BeginDockspaceAndTopBar();

    //Made the movement go up and down , side to side to like a rollercoaster//
    //TODO - Create a control manager class and add this there//
    if (glfwGetKey(window,GLFW_KEY_W) == GLFW_PRESS)
    {
        camera.position.z -= 0.05f;
    }

    if (glfwGetKey(window,GLFW_KEY_S) == GLFW_PRESS)
    {
        camera.position.z += 0.05f;
    }

    if (glfwGetKey(window,GLFW_KEY_A) == GLFW_PRESS)
    {
        camera.position.x -= 0.05f;
    }

    if (glfwGetKey(window,GLFW_KEY_D) == GLFW_PRESS)
    {
        camera.position.x += 0.05f;
    }

    if (glfwGetKey(window,GLFW_KEY_Q) == GLFW_PRESS)
    {
        camera.position.y -= 0.05f;
    }

    if (glfwGetKey(window,GLFW_KEY_E) == GLFW_PRESS)
    {
        camera.position.y += 0.05f;
    }

    ui.Draw(scene, camera, selectedIndex,

        //Meshes (read-only queries)
        [this](const std::string& key){ return meshmanager.Get(key); },
        [this](){ return meshmanager.Keys(); },

        //Textures (read-only queries)
        [this](const std::string& key){ return textureManager.Get(key); },
        [this](){ return textureManager.Keys(); },

        //Push message (the only "action" UI can do)
        [this](std::unique_ptr<Message> m){ PushMessage(std::move(m)); }
    );

    // Message vomit : UI asks, Engine does.
    ProcessMessages();

}

void EngineContext::Render()
{

    renderer.Begin();
    renderer.RenderScene(scene ,camera);
    //Rendering IMGUI honestly should make a UImanager right after this I might need it
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    glfwSwapBuffers(window);//Swapping buffers to make colour come in front on the viewport.

}

void EngineContext::Terminate()
{

    //Destroy or Shut it down //
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    PrintMemoryStatus();

    //FIXME: IMPORTANT - delete GL resources BEFORE context dies//
    meshmanager.Clear();
    textureManager.Clear();

    PrintMemoryStatus();

//Destroy Imgui before window otherwise it can cause various issues later //
    glfwDestroyWindow(window);
    glfwTerminate();



}

void EngineContext::AddObject()
{
    scene.CreateObject();
}

std::vector<SceneObject>& EngineContext::Getobject()
{
    return scene.GetObjects();
}

bool EngineContext::DeleteEntity(Entity e)
{
    auto& objects = scene.GetObjects();

    for (size_t i = 0; i < objects.size(); i++)
    {
        if (objects[i].entity.Id == e.Id) //Entity compare by Id//
        {
            scene.DestroyObjectAt(i);

            //Fix selectedIndex so it doesn't point into garbage//
            int count = (int)scene.GetObjects().size();
            if (count == 0)
            {
                selectedIndex = -1;
            }
            else
            {
                //If you deleted something before the selected index, shift it left//
                if (selectedIndex > (int)i)
                    selectedIndex--;

                if (selectedIndex >= count)
                    selectedIndex = count - 1;
            }

            return true;
        }
    }

    return false;
}
void EngineContext::SetEntityAlbedo(Entity e, const std::string& albedoKey)
{
    for (auto& obj : scene.GetObjects())
    {
        if (obj.entity.Id == e.Id)
        {
            obj.albedoKey = albedoKey;
            obj.albedo = textureManager.Get(albedoKey); // or whatever your getter is
            return;
        }
    }
}

void EngineContext::SetEntitySpecular(Entity e, const std::string& specularKey)
{
    for (auto& obj : scene.GetObjects())
    {
        if (obj.entity.Id == e.Id)
        {
            obj.specularKey = specularKey;
            obj.specular = textureManager.Get(specularKey);
            return;
        }
    }
}

void EngineContext::SetEntityShininess(Entity e, float shininess)
{
    for (auto& obj : scene.GetObjects())
    {
        if (obj.entity.Id == e.Id)
        {
            obj.shininess = shininess;
            return;
        }
    }
}

void EngineContext::SetMipmapsEnabled(bool enabled)
{
    useMipmaps = enabled;

    // apply to all textures currently loaded
    for (const auto& key : textureManager.Keys())
    {
        Texture* t = textureManager.Get(key);
        if (t) t->ApplySampling(useMipmaps);
    }
}
