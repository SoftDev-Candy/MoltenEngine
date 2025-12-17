//
// Created by Candy on 11/21/2025.
//

#include "EngineContext.hpp"

#include "ShaderSource.hpp"
#include"../external/imgui/imgui.h"
#include"../external/imgui/imgui_impl_glfw.h"
#include"../external/imgui/imgui_impl_opengl3.h"


static void framebuffer_size_callback(GLFWwindow* , int w , int h)
{
    glViewport(0,0,w,h);
}

Entity EngineContext::CreateCube(const std::string &cubename)
{
    scene.CreateObject();
    auto& obj = scene.GetObjects().back();
    obj.mesh.mesh = cubeMesh;
    obj.name = cubename;

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

    glViewport(2,4,1000,800); //This is the viewport or the drawable size area

    //Depth test dont worry about it until it throws error....uhh then get a panic attack and! worry//
    //from the future - Dont do the second part ⬆️
    glEnable(GL_DEPTH_TEST);

    //THIS IS IT FINALLY THE COSMIC CREATION OF THE MOTHERLAND STARDENBURDENHARDENBART DEAR IMGUI//
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    //Enabling Docking here
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad; //Enabling Controller dont need it saw it sounds cool might keep it for now;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; //Enabling Keyboard

ImGui::StyleColorsLight();

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

    //FIXME: this is getting too cryptic need to ask for a better solution for this no one
    //is going to understand this jargon //
    cubeMesh= new Mesh(vertices, 40, indices,36);

    //FIXME - NEED A MESH MANAGET TO DELETE THIS //

    shadermanager.LoadShader("Default",VertexShaderSource,FragmentShaderSource);
    renderer.SetActiveShader(shadermanager.GetShader("Default"));

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

    //TODO - Delete this after you have setup the UI and its colours properly its just there for learning purposes//
    ImGui::ShowDemoWindow();

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

ImGui::Begin("Scene Hierarchy");

    auto& objects = scene.GetObjects();
    for (int i =0;i<objects.size();i++)
    {
    bool clicked = ImGui::Selectable(objects[i].name.c_str(),selectedIndex == i);

        if (clicked)
        {
            selectedIndex = i;
        }

    }
    ImGui::End();
    ImGui::Begin("Camera FOV");
    ImGui::SliderFloat("FOV",&camera.fov, 30.0f, 120.0f, "ratio = %.1f");
    ImGui::End();




    float time = static_cast<float>(glfwGetTime());
    if (selectedIndex != -1)
    {
        auto& obj = scene.GetObjects()[selectedIndex];
        obj.transform.rotation.y = time;


        //Grab my object and add it to the UI, so I can manipulate it //
        ImGui::Begin("Inspector Panel");

        //Fixme: Changing name Context here need to change this later to something else//
        char changedBuffer[62] = "";
        strcpy(changedBuffer , obj.name.c_str());

        bool changedName = ImGui::InputText("Name", changedBuffer, sizeof(changedBuffer));
        if(changedName)
        {
            obj.name = changedBuffer;
        }

        //This is how we change the position
        ImGui::DragFloat3("Position",&obj.transform.position.x);

        //This is how we change the Rotation
        ImGui::DragFloat3("Rotation",&obj.transform.rotation.x);

        //This is how we scale it yo//
        ImGui::DragFloat3("Scale",&obj.transform.scale.x);

        ImGui::End();
    }

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
