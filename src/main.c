#define GLEW STATIC
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <pulse/pulseaudio.h>
#include <pulse/simple.h>
#include <semaphore.h>

#include "pulse.h"
#include "AppContext.h"

AppContext *app;

int main()
{
    app = new_AppContext();
    app->windowName = "Shader livecoding tool";
    app->windowWidth = 800;
    app->windowHeight = 600;

    app->init(app);
    app->loop(app);
    app->terminate(app);
    destroy_AppContext(app);
}
