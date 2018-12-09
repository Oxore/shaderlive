typedef struct AppContext {
    GLuint windowWidth;
    GLuint windowHeight;
    GLchar *windowName;
    GLFWwindow *window;
    const char *vertexShaderName;
    const char *fragmentShaderName;
    time_t vertexShaderModTime;
    time_t fragmentShaderModTime;
    GLchar *modelVertexShaderSource;
    GLchar *modelFragmentShaderSource;
    GLuint shader;
    GLuint VAO;
    GLuint VBO;
    GLuint IBO;

    struct audio_data audio;

    GLfloat deltaTime;    // Time between current frame and last frame
    GLfloat lastFrame;   // Time of last frame

    void (*init)(struct AppContext *this);
    void (*loop)(struct AppContext *this);
    void (*terminate)(struct AppContext *this);
} AppContext;

AppContext *new_AppContext();
void AppContext_init(AppContext *app);
void AppContext_loop(AppContext *app);
void AppContext_terminate(AppContext *app);
void destroy_AppContext(AppContext *app);
