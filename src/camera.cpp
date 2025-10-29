#include <camera.hpp>


void windowSizeCallBack(GLFWwindow* window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
}

Camera::Camera():
    mode(VIEWMODE),
    pos(0.0f, 0.0f, 2.0f),
    front(0.0f, 0.0f, -1.0f),
    up(0.0f, 1.0f, 0.0f),
    right(0.0f),
    motionPosDir{0.0f, 0.0f, 0.0f},
    animationFlag{false},
    dir{0},
    yaw{0.0f},
    pitch{0.0f},
    firstCamera{true}
    {
        glfwSetTime(0.0);
        lastTime = 0.0;
    }

void Camera::init()
{
    lastTime = glfwGetTime();
    dir = 0;
    lastCursorPos.x = 0.0f;
    lastCursorPos.y = 0.0f;
    firstCamera = true;
    pitch = 0.0f;
    yaw = 0.0f;
}

void Camera::setUniforms()
{
    glUseProgram(programID);
    GLint loc;
    loc = glGetUniformLocation(programID, "view");
    glm::mat4 viewMat{1.0f};
    viewMat = glm::lookAt(pos, front + pos, up);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(viewMat));

    loc = glGetUniformLocation(programID, "projection");
    glm::mat4 projectionMat{1.0f};
    projectionMat = glm::perspective(rFov, aspect, pNear, pFar);
    glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(projectionMat));
}

void Camera::setViewMode()
{
    pos = ViewModeDefaultPos;
    front = ViewModeDefaultFront;
    up = ViewModeDefaultUp;

    mode = VIEWMODE;
    setUniforms();
}
void Camera::setLandMode()
{
    pos = LandModeDefaultPos;
    front = LandModeDefaultFront;
    up = LandModeDefaultUp;
    right = glm::normalize(glm::cross(front, up));
    mode = LANDMODE;
    firstCamera = true;
    setUniforms();
}
void Camera::setLandMode(float x, float y)
{
    pos = glm::vec3(x, y, LandModeDefaultPos.z);
    front = LandModeDefaultFront;
    up = LandModeDefaultUp;
    right = glm::normalize(glm::cross(front, up));
    mode = LANDMODE;
    firstCamera = true;
    setUniforms();
}
void Camera::switchMode()
{
    if (isViewMode())
    {
        initiateMotion(LandModeDefaultPos, LandModeDefaultFront, LandModeDefaultUp);
    }
    else if (isLandMode())
    {
        initiateMotion(ViewModeDefaultPos, ViewModeDefaultFront, ViewModeDefaultUp);
    }
}
void Camera::switchMode(glm::vec3 motionPosTarget)
{
    if (isViewMode())
    {
        initiateMotion(motionPosTarget, LandModeDefaultFront, LandModeDefaultUp);
    }
    else if (isLandMode())
    {
        initiateMotion(ViewModeDefaultPos, ViewModeDefaultFront, ViewModeDefaultUp);
    }
}
void Camera::initiateMotion(glm::vec3 motionPosTarget, glm::vec3 motionFrontTarget, glm::vec3 motionUpTarget)
{
    if (isViewMode())
    {
        landMotionTargetX = motionPosTarget.x;
        landMotionTargetY = motionPosTarget.y;
    }
    motionPosDir = motionPosTarget - pos;
    motionFrontDir = motionFrontTarget - front;
    motionUpDir = motionUpTarget - up;
    motionPosVel = glm::length(motionPosDir) / AnimationTime;
    motionFrontVel = glm::length(motionFrontDir) / AnimationTime;
    motionUpVel = glm::length(motionUpDir) / AnimationTime;
    motionPosDir = glm::normalize(motionPosDir);
    motionFrontDir = glm::normalize(motionFrontDir);
    motionUpDir = glm::normalize(motionUpDir);
    AnimationStart = glfwGetTime();
    animationFlag = true;
}
bool Camera::isViewMode()
{
    return (mode == VIEWMODE);
}
bool Camera::isLandMode()
{
    return (mode == LANDMODE);
}
bool Camera::isInAnimation()
{
    return (animationFlag);
}
void Camera::fly(float dx, float dy, float dz)
{
    pos.x += dx;
    pos.y += dy;
    pos.z += dz;
    setUniforms();
}
void Camera::walk(float dx, float dy)
{
    pos.x += dx;
    pos.y += dy;
    setUniforms();
}
void Camera::takeStep()
{
    double curTime = glfwGetTime();
    float dt = curTime - lastTime;
    switch (mode)
    {
        case LANDMODE:
        if (dir & DirectionFront)
            pos += dt * landVelX * glm::vec3(front.x, front.y, 0.0f);
        else if (dir & DirectionNFront)
            pos -= dt * landVelX * glm::vec3(front.x, front.y, 0.0f);
        if (dir & DirectionRight)
            pos += dt * landVelY * glm::vec3(right.x, right.y, 0.0f);
        else if (dir & DirectionNRight)
            pos -= dt * landVelY * glm::vec3(right.x, right.y, 0.0f);
        if (dir & DirectionUp)
            pos += dt * landVelZ * glm::vec3(0.0f, 0.0f, 1.0f);
        else if (dir & DirectionNUp)
            pos -= dt * landVelZ * glm::vec3(0.0f, 0.0f, 1.0f);
        break;

        case VIEWMODE:
        if (dir & DirectionX)
            pos.x += dt * viewVelX;
        else if (dir & DirectionNX)
            pos.x -= dt * viewVelX;
        if (dir & DirectionY)
            pos.y += dt * viewVelY;
        else if (dir & DirectionNY)
            pos.y -= dt * viewVelY;
        if (dir & DirectionZ)
            pos.z += dt * viewVelZ;
        else if (dir & DirectionNZ)
            pos.z -= dt * viewVelZ;
        break;
    }
    dir = 0;
}
void Camera::forwardAnimation()
{
    if ( !isInAnimation() )
        return;
    
    double curTime = glfwGetTime();
    double dt = curTime - lastTime;

    if (curTime - AnimationStart > AnimationTime)
    {
        animationFlag = false;
        if (isViewMode())
        {
            setLandMode(landMotionTargetX, landMotionTargetY);
        }
        else if (isLandMode())
        {
            setViewMode();
        }
    }
    else
    {
        pos += (float)dt * motionPosVel * motionPosDir;
        front += (float)dt * motionFrontVel * motionFrontDir;
        up += (float)dt * motionUpVel * motionUpDir;
        signalMotion();
    }
}
void Camera::signalMotion()
{
    moveFlag = true;
}
void Camera::LinkShader(GLuint _programID)
{
    programID = _programID;
}
void Camera::screen2World(double xposIn, double yposIn, float* xposOut, float* yposOut)
{
    /*
    reference: https://www.opengl-tutorial.org/miscellaneous/clicking-on-objects/picking-with-a-physics-library/
    */
    //first, calculate the uniform matrices that were used to convert world coord to NDC coord
    glm::mat4 viewMat{1.0f};
    viewMat = glm::lookAt(pos, front, up);
    glm::mat4 projectionMat{1.0f};
    projectionMat = glm::perspective(glm::radians(45.0f), (float)gWidth / (float)gHeight, 0.1f, 100.0f);
    glm::mat4 M = projectionMat * viewMat;
    glm::mat4 invM = glm::inverse(M);

    // calculate NDC coordinates of ray start and ray end
    glm::vec4 rayStart{
        ((float) xposIn / gWidth - 0.5f) * 2.0f,
        ((float) yposIn / gHeight - 0.5f) * (-2.0f),
        -1.0f,
        1.0f
    };

    glm::vec4 rayEnd{
        ((float) xposIn / gWidth - 0.5f) * 2.0f,
        ((float) yposIn / gHeight - 0.5f) * (-2.0f),
        1.0f,
        1.0f
    };

    glm::vec4 rayStartW = invM * rayStart;
    rayStartW /= rayStartW.w;
    glm::vec4 rayEndW = invM * rayEnd;
    rayEndW /= rayEndW.w;
    float a = (0.0f - rayStartW.z) / (rayEndW.z - rayStartW.z);
    glm::vec4 targetPos = rayStartW + a * (rayEndW - rayStartW);
    *xposOut = targetPos.x;
    *yposOut = targetPos.y;
}

using namespace std;

int camera_demo()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(800, 600, "camera_demo", NULL, NULL);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        cout << "GLAD loading failed" << endl;
        exit(1);
    }

    glfwSetWindowSizeCallback(window, windowSizeCallBack);

    Camera cam;
    glfwSetWindowUserPointer(window, (void*)&cam);

    glViewport(0, 0, 800, 600);

    // set up grid world
    // Let's draw the grid using GL_LINE primitive. This considers 2 vertices as forming a line segment.
    const int N = 40;
    const float L = 3.0;
    // we want to scale the grids so that the entire picture should span about 3 * 3 of the whole screen
    const float scale = L / (float) N;
    float gridGround[2 * 2 * N * 3] = {0.0};
    float* gridPtr;
    for (int i=0; i<N; i++)
    {
        gridPtr = gridGround + 2 * 3 * i;
        *gridPtr = i * scale;
        *(gridPtr + 1) = 0.0f;
        *(gridPtr + 2) = 0.0f;
        *(gridPtr + 3) = i * scale;
        *(gridPtr + 4) = L;
        *(gridPtr + 5) = 0.0f;
    }
    for (int j=0; j<N; j++)
    {
        gridPtr = gridGround + 2 * 3 * N + 2 * 3 * j;
        *gridPtr = 0.0f;
        *(gridPtr + 1) = j * scale;
        *(gridPtr + 2) = 0.0f;
        *(gridPtr + 3) = L;
        *(gridPtr + 4) = j * scale;
        *(gridPtr + 5) = 0.0f;
    }

    // Now the whole picture should span around [0,L) * [0,L), so we slide it
    for (int i=0; i < 2 * 2 * N * 3; i++)
    {
        if (i % 3 != 2)
        gridGround[i] -= 0.5f * L;
    }

    // setting up buffers
    GLuint VAO{0};
    GLuint VBO{0};
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gridGround), gridGround, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    // setting up shaders
    const char* vsSource = "#version 330 core\n"
        "layout (location = 0) in vec3 pos;\n"
        "uniform mat4 view;\n"
        "uniform mat4 projection;\n"
        "void main() {\n"
        "   gl_Position = projection * view * vec4(pos, 1.0f);\n"
        "}\0";
    const char* fsSource = "#version 330 core\n"
        "out vec4 FragColor;\n"
        "void main() {\n"
        "   FragColor = vec4(0.5f, 0.0f, 0.5f, 1.0f);\n"
        "}\0";

    GLint success;
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vShader, 1, &vsSource, NULL);
    glCompileShader(vShader);
    if (glGetShaderiv(vShader, GL_COMPILE_STATUS, &success); !success)
    {
        cout << "vShader compilation failed" << endl;
        exit(1);
    }
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fShader, 1, &fsSource, NULL);
    glCompileShader(fShader);
    if (glGetShaderiv(fShader, GL_COMPILE_STATUS, &success); !success)
    {
        cout << "fShader compilation failed" << endl;
        exit(1);
    }
    cam.programID = glCreateProgram();
    glAttachShader(cam.programID, vShader);
    glAttachShader(cam.programID, fShader);
    glLinkProgram(cam.programID);
    if (glGetProgramiv(cam.programID, GL_LINK_STATUS, &success); !success)
    {
        cout << "Program linking failed" << endl;
        exit(1);
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // pre-render
    glfwSetTime(0.0);
    cam.init();

    // glfwSetKeyCallback(window, keyCallBack);
    glLineWidth(2.0f);
    cam.setUniforms();

    // render loop
    while ( !glfwWindowShouldClose(window) )
    {
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);


        if (cam.moveFlag)
        {
            cam.takeStep();
            cam.setUniforms();
            cam.moveFlag = false;
        }

        glUseProgram(cam.programID);
        glBindVertexArray(VAO);

        glDrawArrays(GL_LINES, 0, 2 * 2 * N);

        glfwSwapBuffers(window);
        glfwPollEvents();
        cam.forwardAnimation();
        cam.processInput(window);

        cam.lastTime = glfwGetTime();
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(1, &VAO);
    glDeleteProgram(cam.programID);
    glfwTerminate();

    return 0;
}
