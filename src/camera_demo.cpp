#include <camera.hpp>

int main()
{
    camera_demo();
}

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
        switchMode();
        return;
    }
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionY;
        else if (isLandMode())
            dir |= DirectionX;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionNY;
        else if (isLandMode())
            dir |= DirectionNX;
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionX;
        else if (isLandMode())
            dir |= DirectionNY;
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
    {
        signalMotion();
        if (isViewMode())
            dir |= DirectionNX;
        else if (isLandMode())
            dir |= DirectionY;
    }
}

void keyCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    Camera* camera = (Camera*)glfwGetWindowUserPointer(window);
    if (camera->isInAnimation())
        return;
    DirectionEnum dir = 0;
    if (action != GLFW_PRESS && action != GLFW_REPEAT)
        return;
    switch (key)
    {
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        case GLFW_KEY_UP:
            camera->signalMotion();
            if (camera->isViewMode())
                camera->dir |= DirectionY;
            else if (camera->isLandMode())
                camera->dir |= DirectionX;
            break;
        case GLFW_KEY_DOWN:
            camera->signalMotion();
            if (camera->isViewMode())
                camera->dir |= DirectionNY;
            else if (camera->isLandMode())
                camera->dir |= DirectionNX;
            break;
        case GLFW_KEY_RIGHT:
            camera->signalMotion();
            if (camera->isViewMode())
                camera->dir |= DirectionX;
            else if (camera->isLandMode())
                camera->dir |= DirectionNY;
            break;
        case GLFW_KEY_LEFT:
            camera->signalMotion();
            if (camera->isViewMode())
                camera->dir |= DirectionNX;
            else if (camera->isLandMode())
                camera->dir |= DirectionY;
            break;
        case GLFW_KEY_E:
            camera->switchMode();
            break;
        default:
            break;
    }
}
