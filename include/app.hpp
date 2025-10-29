#pragma once

#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <game.hpp>

constexpr int gWidth{ 800 };
constexpr int gHeight{ 600 };
constexpr double SecPerFrame{ 0.5 };

typedef void (*wsCallbackPtr) (GLFWwindow*, int, int);
typedef void (*kpCallbackPtr) (GLFWwindow*, int, int, int, int);
typedef void (*cspCallbackPtr) (GLFWwindow*, double, double);
typedef void (*mbCallbackptr) (GLFWwindow*, int, int, int);

void appWindowSizeCallBack(GLFWwindow* window, int width, int height);
void appKeyPressCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

const float quadVerticesPos[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.0, 0.0
};

// very simple app class that only handles fixed-sized arrays
class App
{
    public:
    Game game;
    App(ui dimX, ui dimY, int seed);
    /// @brief initialize the app (including GLAD, window)
    void init();
    virtual void run();

    protected:
    GLFWwindow* window;
    GLuint VAO[MAX_ROW * MAX_COL], VBO, CellLBO, BTO;
    GLuint programID, texID;
    ui dimX, dimY;
    double lastTime;
    /// @brief seconds per state update. Separate from the frame refresh rate
    double gameTick = 1.0;
    const double gameTickIncre = 0.05;
    /// @brief time when the last state update occurred
    double lastStateUpdate;
    void setCallbacks(wsCallbackPtr, kpCallbackPtr, cspCallbackPtr, mbCallbackptr);
    virtual void setCallbacks();
    void setupBufferTextureObject();
    void setupVertexBufferObject();
    void writeToBufferTexture(StateType cellState[MAX_ROW][MAX_COL]);
    void setupShader(const char* vShaderSourcePath, const char* fShaderSourcePath);
    virtual void preRender();
    virtual void render();
    void quit();
    void changeSimSpeed(int c);
};