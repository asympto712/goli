#include <goli.hpp>
#include <utils.hpp>
#include "gifenc/gifenc.h"

void GAwindowSizeCallback(GLFWwindow* window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
    glViewport(0, 0, width, height);
}

void GoliApp::run()
{
    init();
    setupVertexBufferObject();
    setupBufferTextureObject();
    setupShader("src/shader/goli.vs", "src/shader/goli.fs");
    cam.LinkShader(programID);
    preRender();
    while (!glfwWindowShouldClose(window))
    {
        render();
    }
    quit();
}
void GoliApp::runWithRec(ge_GIF* gif)
{
    uint8_t _frameRGB[3 * gWidth * gHeight];
    double lastGifWrite;
    double curtime;
    const double gifWriteTimeout = 0.16;

    init();
    setupVertexBufferObject();
    setupBufferTextureObject();
    setupShader("src/shader/goli.vs", "src/shader/goli.fs");
    cam.LinkShader(programID);
    preRender();
    // set the initial gif write time
    lastGifWrite = glfwGetTime();

    while (!glfwWindowShouldClose(window))
    {
        render();
        if (curtime = glfwGetTime(); curtime - lastGifWrite > gifWriteTimeout)
        {
            // pixel data is returned in row order from the lowest to the highest row, left to right in each row.
            glReadPixels(0, 0, gWidth, gHeight, GL_RGB, GL_UNSIGNED_BYTE, (void*) _frameRGB);
            for (ui i=0; i<gHeight; i++)
            {
                for (ui j=0; j<gWidth; j++)
                {
                    // gifenc library expects one-byte color format (16 VGA colors + 216 web-safe colors + 24 grey colors).
                    // Since rgb2oneb outputs the location in the 216 web-safe color table, we need to add the offset of 16
                    // Also, it expects the data to be read from the highest row to the lowest
                    ui frameIndex = i * gWidth + j;
                    gif->frame[(gHeight - 1 - i) * gWidth + j] = 
                    16 + rgb2oneb(_frameRGB[3 * frameIndex], _frameRGB[3 * frameIndex + 1], _frameRGB[3 * frameIndex + 2]);
                }
            }
            ge_add_frame(gif, 16);
            lastGifWrite = curtime;
        }
    }
    quit();
}

void GoliApp::init()
{
    App::init();
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}
void GoliApp::preRender()
{
    glUseProgram(programID);
    GLint loc;
    loc = glGetUniformLocation(programID, "sb");
    glUniform1i(loc, 0);
    glfwSetTime(0.0);
    updateLastTime();
    cam.init();
    cam.setUniforms();

    // just to make sure
    paused = false;

    lastKeyPress = glfwGetTime();
}
void GoliApp::render()
{
    writeToBufferTexture(game.state);

    glClearColor(0.5f, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    if (cam.moveFlag)
    {
        cam.takeStep();
        cam.setUniforms();
        cam.moveFlag = false;
    }

    glBindVertexArray(VAO[0]);
    glUseProgram(programID);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_BUFFER, texID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, BTO);
    glDrawArrays(GL_TRIANGLES, 0, 6 * dimX * dimY);

    glfwSwapBuffers(window);
    glfwPollEvents();
    processInput();
    cam.forwardAnimation();
    cam.processInput(window);

    if (!paused)
    {
        double curTime = glfwGetTime();
        double timeDiff = curTime - lastStateUpdate;
        if (timeDiff >= gameTick)
        {
            game.step();
            lastStateUpdate = curTime;
        }
    }
    
    updateLastTime();
}
void GoliApp::updateLastTime()
{
    lastTime = glfwGetTime();
    cam.lastTime = lastTime;
}
void GoliApp::setCallbacks()
{
    App::setCallbacks(GAwindowSizeCallback, GAKeyPressCallback, GACursorPosCallback, GAMouseButtonCallback);
}

/// @brief handles input that is appropriate to be processed by Camera alone (not directly concerned with global game/app state)
/// @param window current context window
void Camera::processInput(GLFWwindow* window)
{
    // during the animation, do not allow user input
    if (isInAnimation())
        return;
    if (glfwGetKey(window, GLFW_KEY_Q) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_E) == GLFW_PRESS)
    {
        // when mode change is ordered, accept no other input
        double xpos, ypos;
        glfwGetCursorPos(window, &xpos, &ypos);
        float wxpos, wypos;
        screen2World(xpos, ypos, &wxpos, &wypos);
        switchMode(glm::vec3(wxpos, wypos, 0.0f));
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionY;
        else if (isLandMode())
            dir |= DirectionFront;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionNY;
        else if (isLandMode())
            dir |= DirectionNFront;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionX;
        else if (isLandMode())
            dir |= DirectionRight;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionNX;
        else if (isLandMode())
            dir |= DirectionNRight;
    }

    if (isLandMode())
    {
        double xPos, yPos;
        glfwGetCursorPos(window, &xPos, &yPos);
        if (firstCamera)
        {
            firstCamera = false;
            lastCursorPos.x = xPos;
            lastCursorPos.y = yPos;
            return;
        }
        float diffX = (float) xPos - lastCursorPos.x;
        float diffY = lastCursorPos.y - (float) yPos;
        // movement across whole screen width/height should be roughly 180 degrees movement
        diffX /= (float) gWidth;
        diffX *= 180.0f;
        diffY /= (float) gHeight;
        diffY *= 180.0f;

        yaw += diffX;
        pitch += diffY;

        if (pitch > 89.0f)
            pitch = 89.0f;
        if (pitch < -89.0f)
            pitch = -89.0f;

        // they accept radians
        front.z = sinf(glm::radians(pitch));
        front.x = cosf(glm::radians(pitch)) * cosf(glm::radians(yaw));
        front.y = cosf(glm::radians(pitch)) * - sinf(glm::radians(yaw));
        front = glm::normalize(front);
        right = glm::normalize(glm::cross(front, up));
        lastCursorPos.x = xPos;
        lastCursorPos.y = yPos;
        signalMotion();
    }
}

/// @brief Handles app-side input, or any input that should not be handled solely by Camera
void GoliApp::processInput()
{
    // flip the pause flag
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
    {
        double curTime = glfwGetTime();
        if (curTime - lastKeyPress > keyInputTimeout)
        {
            lastKeyPress = curTime;
            if (paused)
            {
                paused = false;
                lastStateUpdate = glfwGetTime();
            }
            else paused = true;
        }
    }

    // change simulation speed
    if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
    {
        double curTime = glfwGetTime();
        if (curTime - lastKeyPress > keyInputTimeout)
        {
            lastKeyPress = curTime;
            changeSimSpeed(1);
        }
    }
    if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
    {
        double curTime = glfwGetTime();
        if (curTime - lastKeyPress > keyInputTimeout)
        {
            lastKeyPress = curTime;
            changeSimSpeed(-1);
        }
    }
}

void GAKeyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    
}

void GACursorPosCallback(GLFWwindow* window, double xpos, double ypos)
{

}

void GAMouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{

}