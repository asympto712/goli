#pragma once

#define VIEWMODE 1
#define LANDMODE 2

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <cstdint>
#include <app.hpp>

typedef uint8_t DirectionEnum;
constexpr uint8_t DirectionX{ 0b00000001 };
constexpr uint8_t DirectionY{ 0b00000010 };
constexpr uint8_t DirectionZ{ 0b00000100 };
constexpr uint8_t DirectionNX{ 0b00001000 };
constexpr uint8_t DirectionNY{ 0b00010000 };
constexpr uint8_t DirectionNZ{ 0b00100000 };
// Land mode specific
constexpr uint8_t DirectionFront{ 0b00000001 };
constexpr uint8_t DirectionRight{ 0b00000010 };
constexpr uint8_t DirectionUp{ 0b00000100 };
constexpr uint8_t DirectionNFront{ 0b00001000 };
constexpr uint8_t DirectionNRight{ 0b00010000 };
constexpr uint8_t DirectionNUp{ 0b00100000 };

int camera_demo();

void windowSizeCallBack(GLFWwindow* window, int width, int height);
void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

class Camera
{
    public:
    /// @brief the shader this camera is linked to
    GLuint programID;
    int mode;
    glm::vec3 pos;
    glm::vec3 front;
    glm::vec3 up;
    glm::vec3 right;
    bool moveFlag;
    double lastTime;
    double AnimationStart;
    Camera();
    void init();
    /// @brief send the updated uniform to render and turn off the move flag
    void setUniforms();
    /// @brief initiate the motion sequence using only the default target
    void switchMode();
    void switchMode(glm::vec3 motionPosTarget);
    void setViewMode();
    void setLandMode();
    void setLandMode(float x, float y);
    bool isViewMode();
    bool isLandMode();
    bool isInAnimation();
    void fly(float dx, float dy, float dz);
    void walk(float dx, float dy);
    void takeStep();
    /// @brief if the animation flag is on, move the animation sequence one step forward
    void forwardAnimation();
    /// @brief signal the rendering loop that a movement occurred (and therefore the uniform update is needed)
    void signalMotion();
    void processInput(GLFWwindow* window);
    void LinkShader(GLuint _programID);

    private:

    glm::vec2 lastCursorPos;
    const float mouseSensitivity = 0.1;
    /// @brief in degrees (not radians)
    float pitch, yaw;
    bool firstCamera;

    DirectionEnum dir;
    /// @brief boolean flag to indicate that the character is in the animation sequence
    bool animationFlag;
    /// @brief motion movement direction for the camera position
    glm::vec3 motionPosDir;
    /// @brief motion movement direction for the camera front
    glm::vec3 motionFrontDir;
    /// @brief motion movement direction for the camera up
    glm::vec3 motionUpDir;
    /// @brief motion velocity for the camera position
    float landMotionTargetX, landMotionTargetY;
    float motionPosVel;
    /// @brief motion velocity for the camera front
    float motionFrontVel;
    /// @brief motion velocity for the camera up
    float motionUpVel;
    const float landVelX = 10.0;
    const float landVelY = 10.0;
    const float landVelZ = 10.0;
    const float viewVelX = 20.0;
    const float viewVelY = 20.0;
    const float viewVelZ = 20.0;
    const glm::vec3 ViewModeDefaultPos = glm::vec3{0.0f, 0.0f, 2.0f};
    const glm::vec3 ViewModeDefaultFront = glm::vec3{0.0f, 0.0f, -1.0f};
    const glm::vec3 ViewModeDefaultUp = glm::vec3{0.0f, 1.0f, 0.0f};
    const glm::vec3 LandModeDefaultPos = glm::vec3{0.0f, 0.0f, 0.05f};
    const glm::vec3 LandModeDefaultFront = glm::vec3{1.0f, 0.0f, 0.0f};
    const glm::vec3 LandModeDefaultUp = glm::vec3{0.0f, 0.0f, 1.0f};
    const float AnimationTime = 3.0;
    void initiateMotion(glm::vec3 motionPosTarget, glm::vec3 motionFrontTarget, glm::vec3 motionUpTarget);
    void screen2World(double xposIn, double yposIn, float* xposOut, float* yposOut);
    friend void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods);

    // perspective
    float rFov = glm::radians(45.0f);
    float aspect = (float) gWidth / (float) gHeight;
    const float pNear = 0.01f;
    const float pFar = 50.0f;
};