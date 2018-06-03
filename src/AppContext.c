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
#include <pulse/simple.h>
#include <pulse/mainloop.h>
#include <pulse/error.h>
#include <pulse/pulseaudio.h>
#include <pthread.h>
#include <fftw3.h>

#define M 2048
int sourceIsAuto = 1;
int thr_id;
pthread_t p_thread;
fftw_complex outl[M / 2 + 1][2];
fftw_complex outr[M / 2 + 1][2];
double inl[2 * (M / 2 + 1)];
double inr[2 * (M / 2 + 1)];
fftw_plan pl;
fftw_plan pr;

#include "pulse.h"
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
static void initPulseAudio(AppContext *app);
static void loadShaders(AppContext *app);
static void unloadShaders(AppContext *app);
static void compileShaders(AppContext *app);
static GLuint compileShader(const GLchar *vertSource, const GLchar *fragSource);
static char *textFileRead(const char *filename);
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
    initPulseAudio(app);
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

static void unloadShaders(AppContext *app)
{
    free(app->modelVertexShaderSource);
    free(app->modelFragmentShaderSource);
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
    float *uv2_res = malloc(2 * sizeof(float));
    while(!glfwWindowShouldClose(app->window))
    {
        if (fileIsModified(app->vertexShaderName, app->vertexShaderModTime) ||
            fileIsModified(app->fragmentShaderName, app->fragmentShaderModTime)) {
            app->vertexShaderModTime = getTimeModified(app->vertexShaderName);
            app->fragmentShaderModTime = getTimeModified(app->fragmentShaderName);
            unloadShaders(app);
            loadShaders(app);
            compileShaders(app);
            glUseProgram(app->shader);
        }
        glfwGetWindowSize(app->window, &width, &height);
        uv2_res[0] = width;
        uv2_res[1] = height;
        glViewport(0, 0, width, height);
        GLfloat currentFrame = glfwGetTime();
        app->deltaTime = currentFrame - app->lastFrame;
        app->lastFrame = currentFrame;
        for (int i = 0; i < M; ++i) {
            inl[i] = app->audio.audio_out_l[i];
            inr[i] = app->audio.audio_out_r[i];
        }
        float out_uf_l[M / 2 + 1];
        float out_uf_r[M / 2 + 1];
        for (int i = 0; i < M / 2 + 1; ++i) {
            out_uf_l[i] = (float) outl[i][0][0];
            out_uf_r[i] = (float) outr[i][0][0];
        }
        fftw_execute(pl);
        fftw_execute(pr);
        glUniform2fv(glGetUniformLocation(app->shader, "uv2_res"), 1, uv2_res);
        glUniform1f(glGetUniformLocation(app->shader, "uf_time"), currentFrame);
        glUniform1fv(glGetUniformLocation(app->shader, "uf_fft_l"), M, out_uf_l);
        glUniform1fv(glGetUniformLocation(app->shader, "uf_fft_r"), M, out_uf_r);
        glfwPollEvents();
        glClear(GL_COLOR_BUFFER_BIT |
                GL_DEPTH_BUFFER_BIT |
                GL_STENCIL_BUFFER_BIT);
        glBindVertexArray(app->VAO);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        glBindVertexArray(0);
        glfwSwapBuffers(app->window);
    }
    free(uv2_res);
}

static void initPulseAudio(AppContext *app)
{
    //input: init
    app->audio.source = malloc(1 +  strlen("auto"));
    strcpy(app->audio.source, "auto");

    app->audio.format = -1;
    app->audio.rate = 0;
    app->audio.terminate = 0;
    app->audio.channels = 2;

    for (int i = 0; i < M; i++) {
        app->audio.audio_out_l[i] = 0;
        app->audio.audio_out_r[i] = 0;
    }

    if (strcmp(app->audio.source, "auto") == 0) {
        getPulseDefaultSink((void*)&app->audio);
        sourceIsAuto = 1;
    } else
        sourceIsAuto = 0;
    //starting pulsemusic listener
    thr_id = pthread_create(&p_thread, NULL, input_pulse, (void*)&app->audio);
    app->audio.rate = 44100;
    //fft: planning to rock
    pl = fftw_plan_dft_r2c_1d(M, inl, *outl, FFTW_MEASURE);
    pr = fftw_plan_dft_r2c_1d(M, inr, *outr, FFTW_MEASURE);

}

void AppContext_terminate(AppContext *app)
{
    glfwDestroyWindow(app->window);
    glfwTerminate();
}

void destroy_AppContext(AppContext *app)
{
    glDeleteVertexArrays(1, &app->VAO);
    glDeleteBuffers(1, &app->VBO);
    glDeleteBuffers(1, &app->IBO);
    unloadShaders(app);

    free(app);
}

static char *textFileRead(const char *filename){
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
    if (err != 0)
        return 0;
    return file_stat.st_mtime > oldMTime;
}

static time_t getTimeModified(const char *name)
{
    struct stat file_stat;
    int err = stat(name, &file_stat);
    if (err != 0)
        return 0;
    return file_stat.st_mtime;
}
