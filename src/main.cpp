#include<iostream>

#include "Cube.hpp"
#include "EngineContext.hpp"
#include "Triangle.hpp"

static void framebuffer_size_callback(GLFWwindow* , int w , int h)
{
    glViewport(0,0,w,h);
}

int main(void)
{

EngineContext EngineObject;
EngineObject.init();
EngineObject.AddObject(new Cube());

    //Its while loop to keep that window open//
while (!EngineObject.ShouldClose())
{

EngineObject.update();
EngineObject.Render();

}
EngineObject.Terminate();

}



