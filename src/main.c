#include <SOIL/SOIL.h>
#define GLEW STATIC
#include <time.h>
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "AppContext.h"

AppContext *app;

int main(int argc, char **argv) {
    app = new_AppContext();
    app->windowName = "Shader livecoding tool";
    app->windowWidth = 800;
    app->windowHeight = 600;

    app->init(app);
    app->loop(app);
    app->terminate(app);
    destroy_AppContext(app);
}
