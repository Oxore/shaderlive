#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <sys/stat.h>
#include <errno.h>
#include <SOIL/SOIL.h>
#define GLEW STATIC
#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "AppContext.h"
#include "ConsoleColor.h"

AppContext *new_AppContext()
{
    AppContext *app = malloc(sizeof(AppContext));
    app->init = AppContext_init;
    app->loop = AppContext_loop;
    app->terminate = AppContext_terminate;

    app->deltaTime = 0.0f;
    app->lastFrame = 0.0f;
    return app;
}

static void initGLFW(AppContext *app);
static void initGLEW(AppContext *app);
static void initVAO(AppContext *app);
static void loadShaders(AppContext *app);
static void compileShaders(AppContext *app);
static GLuint compileShader(const GLchar *vertSource, const GLchar *fragSource);
static const char *textFileRead(const char *filename);
static int fileIsModified(const char *name, time_t oldMTime);
static time_t getTimeModified(const char *name);

void AppContext_init(AppContext *app)
{
    app->vertexShaderName = "shaders/shader.vert";
    app->fragmentShaderName = "shaders/shader.frag";
    initGLFW(app);
    initGLEW(app);
    initVAO(app);
    loadShaders(app);
    compileShaders(app);
}

static void initGLFW(AppContext *app)
{
    glfwInit();
    glfwWindowHint(GLFW_SAMPLES, 256);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE); //For MacOS

    // Create window
    app->window = glfwCreateWindow(app->windowWidth,
            app->windowHeight,
            app->windowName,
            0, 0);
    if (app->window == 0) {
        printf(A_RED "Error: " A_RESET "Failed to Create GLFW window\n");
        glfwTerminate();
        exit(-1);
    }
    glfwMakeContextCurrent(app->window);
}

static void initGLEW(AppContext *app)
{
    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        printf(A_RED "Error: " A_RESET "Failed to initialize GLEW\n");
        exit(-1);
    }
    glClearDepth(1.0f);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
}

static void initVAO(AppContext *app)
{
    GLfloat vertices[] = {
         1.f,  1.f, 0.0f,
         1.f, -1.f, 0.0f,
        -1.f,  1.f, 0.0f,
        -1.f,  -1.f, 0.0f
    };
    GLuint indices[] = {
        0, 1, 2, 1, 2, 3
    };
    glGenVertexArrays(1, &app->VAO);
    glGenBuffers(1, &app->VBO);
    glGenBuffers(1, &app->IBO);
    glBindVertexArray(app->VAO);
    glBindBuffer(GL_ARRAY_BUFFER, app->VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, app->IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
    // Pisistion attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(GLfloat), (GLvoid*)0);
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

static void loadShaders(AppContext *app)
{
    app->modelVertexShaderSource
        = textFileRead(app->vertexShaderName);
    app->modelFragmentShaderSource
        = textFileRead(app->fragmentShaderName);
}

static void compileShaders(AppContext *app)
{
    app->shader = compileShader(app->modelVertexShaderSource,
                                     app->modelFragmentShaderSource);
}


static GLuint compileShader(const GLchar *vertSource, const GLchar *fragSource)
{
    GLint success;
    GLchar infoLog[512];
    GLuint shader;
    // Vertex shader
    GLuint vertShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertShader, 1, &vertSource, 0);
    glCompileShader(vertShader);
    glGetShaderiv(vertShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertShader, 512, 0, infoLog);
        printf( A_RED "Error: " A_RESET
                "Vertex Shader Compilation Failed!\n\t"
                A_YELLOW "^~~~" A_RESET " %s\n", infoLog);
    }
    // Fragment shader
    GLuint fragShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragShader, 1, &fragSource, 0);
    glCompileShader(fragShader);
    glGetProgramiv(fragShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragShader, 512, 0, infoLog);
        printf( A_RED "Error: " A_RESET
                "Fragment Shader Compilation Failed!\n\t"
                A_YELLOW "^~~~" A_RESET " %s\n", infoLog);
    }
    // Link shaders
    shader = glCreateProgram();
    glAttachShader(shader, vertShader);
    glAttachShader(shader, fragShader);
    glLinkProgram(shader);
    // Check for linking errors
    glGetProgramiv(shader, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader, 512, 0, infoLog);
        printf( A_RED "Error: " A_RESET
                "Shader Program Linking Failed!\n\t"
                A_YELLOW "^~~~" A_RESET " %s\n", infoLog);
    }
    glDeleteShader(vertShader);
    glDeleteShader(fragShader);
    return shader;
}

void AppContext_loop(AppContext *app)
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glEnable(GL_MULTISAMPLE);
    int width, height;
    float av2_pos[] = {0.0, 0.0};
    float *uv2_res = malloc(2 * sizeof(float));
    while(!glfwWindowShouldClose(app->window))
    {
        glfwGetWindowSize(app->window, &width, &height);
        glViewport(0, 0, width, height);
        uv2_res[0] = width;
        uv2_res[1] = height;
        if (fileIsModified(app->vertexShaderName, app->vertexShaderModTime) ||
            fileIsModified(app->fragmentShaderName, app->fragmentShaderModTime)) {
            app->vertexShaderModTime = getTimeModified(app->vertexShaderName);
            app->fragmentShaderModTime = getTimeModified(app->fragmentShaderName);
            loadShaders(app);
            compileShaders(app);
        }
        GLfloat currentFrame = glfwGetTime();
        app->deltaTime = currentFrame - app->lastFrame;
        app->lastFrame = currentFrame;
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
        glUseProgram(app->shader);
        glUniform1f(glGetUniformLocation(app->shader, "uf_time"), glfwGetTime());
        glUniform2fv(glGetUniformLocation(app->shader, "av2_pos"), 1, av2_pos);
        glUniform2fv(glGetUniformLocation(app->shader, "uv2_res"), 1, uv2_res);
        glBindVertexArray(app->VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glfwSwapBuffers(app->window);
    }
}
void AppContext_terminate(AppContext *app)
{
    glfwTerminate();
}

void destroy_AppContext(AppContext *app)
{
    glDeleteVertexArrays(1, &app->VAO);
    glDeleteBuffers(1, &app->VBO);
    glDeleteBuffers(1, &app->IBO);
    free(app);
}

static const char *textFileRead(const char *filename){
    FILE *fp;
    char *textFile;
    int textFileSize;
    // Open file
    fp = fopen(filename, "r");
    if (!fp) return 0;
    // Find out file length
    fseek(fp, 0L, SEEK_END);
    textFileSize = ftell(fp);
    fseek(fp, 0L, SEEK_SET);
    // Allocate necessary space
    textFile = malloc(textFileSize+1);
    // Read file into string and put '\0' to end
    fread(textFile, textFileSize, 1, fp);
    textFile[textFileSize] = '\0';
    fclose(fp);
    return textFile;
}

static int fileIsModified(const char *name, time_t oldMTime)
{
    struct stat file_stat;
    int err = stat(name, &file_stat);
    if (err != 0) {
        return 0;
    }
    return file_stat.st_mtime > oldMTime;
}

static time_t getTimeModified(const char *name)
{
    struct stat file_stat;
    int err = stat(name, &file_stat);
    if (err != 0) {
        return 0;
    }
    return file_stat.st_mtime;
}
