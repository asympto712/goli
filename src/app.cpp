
#include <app.hpp>

int basic_app_demo()
{
    App app{50, 50, 9};
    app.run();
    return 0;
}

App::App(ui dimX, ui dimY, int seed):
    dimX(dimX),
    dimY(dimY),
    game(dimX, dimY, seed),
    gameTick{1.0}
    {}

void App::run()
{
    init();
    setupVertexBufferObject();
    setupBufferTextureObject();
    setupShader("src/shader/app.vs", "src/shader/app.fs");
    preRender();
    while( !glfwWindowShouldClose(window) )
    {
        render();
    }
    quit();
}
void App::setCallbacks(wsCallbackPtr wscbFun, kpCallbackPtr kpcbFun, cspCallbackPtr cspcbFun, mbCallbackptr mbcbFun)
{
    glfwSetWindowSizeCallback(window, wscbFun);
    glfwSetKeyCallback(window, kpcbFun);
    glfwSetCursorPosCallback(window, cspcbFun);
    glfwSetMouseButtonCallback(window, mbcbFun);
}
void App::setCallbacks()
{
    setCallbacks(appWindowSizeCallBack, appKeyPressCallBack, NULL, NULL);
}

void App::init()
{
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    window = glfwCreateWindow(gWidth, gHeight, "Game of Life Interactive(GoLI)", NULL, NULL);

    // set callbacks
    setCallbacks();

    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "GLAD load failed" << std::endl;
    }

    glViewport(0, 0, gWidth, gHeight);

    // initialize game
    game.init();
    game.randomPlantSeed(0.5);

    glfwSetTime(0.0);
    lastTime = glfwGetTime();
    lastStateUpdate = lastTime;
}

void App::setupBufferTextureObject()
{
    glBindVertexArray(VAO[0]);
    glGenBuffers(1, &BTO);
    glBindBuffer(GL_TEXTURE_BUFFER, BTO);
    // reserve the memory
    glBufferData(GL_TEXTURE_BUFFER, dimX * dimY * sizeof(StateType), nullptr, GL_DYNAMIC_DRAW);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, BTO);

    // create the texture object, and bind to BTO
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_BUFFER, texID);

}

void App::writeToBufferTexture(StateType cellState[MAX_ROW][MAX_COL])
{
    glBindBuffer(GL_TEXTURE_BUFFER, BTO);
    for (ui i=0; i<dimX; i++)
    {
        glBufferSubData(GL_TEXTURE_BUFFER, i * dimY * 4, dimY * 4, &cellState[i]);
    }

    // very wacky, is there a better way?
    // Okay, seems like glMapBuffer does not work for GL_TEXTURE_BUFFER as target

    // ui* mappedArr = NULL;
    // mappedArr = (ui*) glMapBuffer(GL_TEXTURE_BUFFER, GL_READ_WRITE);
    // if (mappedArr == NULL)
    // {
    //     std::cout << "ERROR::GL Texture buffer could not be mapped to the client memory space" << std::endl;
    //     quit();
    //     return;
    // }
    // for (ui i=0; i<dimX; i++)
    // {
    //     for (ui j=0; j<dimY; j++)
    //     {
    //         /*
    //         if alive, draw in white
    //         else, draw in black
    //         RGBA and each component should take 1 byte
    //         */
    //         if (cellState[i][j] == 0)
    //             for (ui k=0; k<4; k++)
    //                 *(mappedArr + i * dimX * 4 + j * 4 + k) = 0;
    //         else
    //             for (ui k=0; k<4; k++)
    //                 *(mappedArr + i * dimX * 4 + j * 4 + k) = 255;
    //     }
    // }
    // glUnmapBuffer(GL_TEXTURE_BUFFER);
}

void App::setupVertexBufferObject()
{
    // store the cell quad vertices positions

    // First, create a temporary array to store vertices locations
    float* quadLoc = new float[3 * 6 * dimX * dimY];
    float lbX, lbY; // coordinate of left-bottom corner of a quad
    float sizeX = 1.8f / (float) dimX;
    float sizeY = 1.8f / (float) dimY;
    for (ui i=0; i<dimX; i++)
    {
        for (ui j=0; j<dimY; j++)
        {
            for (ui k=0; k < 3 * 6; k++)
            {
                float val;
                switch (k % 3)
                {
                    case 0:
                        val = ((float)i + quadVerticesPos[k]) * sizeX - 0.9f;
                        break;
                    case 1:
                        val = ((float)j + quadVerticesPos[k]) * sizeY - 0.9f;
                        break;
                    default:
                        val = quadVerticesPos[k];
                        break;
                }
                quadLoc[i * 3 * 6 * dimY + j * 3 * 6 + k] = val;
            }
        }
    }
    glGenVertexArrays(1, VAO);
    glGenBuffers(1, &VBO);
    glBindVertexArray(VAO[0]);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, 3 * 6 * dimX * dimY * sizeof(float), quadLoc, GL_STATIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    delete[] quadLoc;

    // store the cell location buffer. This will be used to retrieve the cell state (color)

    /* texelFetch will use texel space to specify the location in the texture buffer.
    This space has the size [0, size) where size = (the total size of the buffer in byte) / (size of the internal format in bytes = 4 in this case).
    */
    GLint* cellLoc = new GLint[6 * dimX * dimY]; // for each cell, we need the total of 6 vertices
    for (ui i=0; i<dimX; i++)
    {
        for (ui j=0; j<dimY; j++)
        {
            for (ui k=0; k<6; k++)
            {
                cellLoc[i * dimY * 6 + j * 6 + k] = (GLint)  i * dimY + j;
            }
        }
    }
    glGenBuffers(1, &CellLBO);
    glBindVertexArray(VAO[0]);

    glBindBuffer(GL_ARRAY_BUFFER, CellLBO);
    glBufferData(GL_ARRAY_BUFFER, 6 * dimX * dimY * sizeof(GLint), cellLoc, GL_STATIC_READ);
    glVertexAttribIPointer(1, 1, GL_INT, sizeof(GLint), (void*)0);
    glEnableVertexAttribArray(1);
    delete[] cellLoc;
}

void App::setupShader(const char* vShaderSourcePath, const char* fShaderSourcePath)
{
    // read from file
    std::ifstream file{};
    file.open(vShaderSourcePath);
    std::stringstream ShaderSS;
    ShaderSS << file.rdbuf();
    std::string vShaderCode;
    const char* vShaderSource;
    vShaderCode = ShaderSS.str();
    vShaderSource = vShaderCode.c_str();
    file.close();

    ShaderSS.str("");
    ShaderSS.clear();
    file.open(fShaderSourcePath);
    ShaderSS << file.rdbuf();
    std::string fShaderCode;
    const char* fShaderSource;
    fShaderCode = ShaderSS.str();
    fShaderSource = fShaderCode.c_str();
    file.close();

    // create shaders
    GLint success;
    char infoLog[256];
    GLsizei logLen;
    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(vShader, 1, &vShaderSource, nullptr);
    glCompileShader(vShader);
    if (glGetShaderiv(vShader, GL_COMPILE_STATUS, &success); !success)
    {
        glGetShaderInfoLog(vShader, 256, &logLen, infoLog);
        std::cout << "ERROR::VShader compilation failed. log msg: " << infoLog << std::endl;
    }
    glShaderSource(fShader, 1, &fShaderSource, nullptr);
    glCompileShader(fShader);
    if (glGetShaderiv(fShader, GL_COMPILE_STATUS, &success); !success)
    {
        glGetShaderInfoLog(fShader, 256, &logLen, infoLog);
        std::cout << "ERROR::FShader compilation failed. log msg: " << infoLog << std::endl;
    }
    programID = glCreateProgram();
    glAttachShader(programID, vShader);
    glAttachShader(programID, fShader);
    glLinkProgram(programID);
    if (glGetProgramiv(programID, GL_LINK_STATUS, &success); !success)
    {
        glGetProgramInfoLog(programID, 256, &logLen, infoLog);
        std::cout << "ERROR::program linking failed. log msg: " << infoLog << std::endl;
    }

    glDeleteShader(vShader);
    glDeleteShader(fShader);
}

void App::preRender()
{
    glUseProgram(programID);
    GLint loc;
    loc = glGetUniformLocation(programID, "sb");
    glUniform1i(loc, 0);
}

void App::render()
{
    writeToBufferTexture(game.state);

    glClearColor(0.5f, 0.5f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glBindVertexArray(VAO[0]);
    glUseProgram(programID);
    glActiveTexture(GL_TEXTURE0 + 0);
    glBindTexture(GL_TEXTURE_BUFFER, texID);
    glTexBuffer(GL_TEXTURE_BUFFER, GL_RGBA8, BTO);
    glDrawArrays(GL_TRIANGLES, 0, 6 * dimX * dimY);

    glfwPollEvents();
    glfwSwapBuffers(window);

    double curTime = glfwGetTime();
    double timeDiff = curTime - lastStateUpdate;
    if (timeDiff >= gameTick)
    {
        game.step();
        lastStateUpdate = curTime;
    }
    
    lastTime = curTime;
}

void App::quit()
{
    glDeleteBuffers(1, &VBO);
    glDeleteBuffers(1, &BTO);
    glfwTerminate();
}

void App::changeSimSpeed(int c)
{
    gameTick += (double) c * gameTickIncre;
}

void appKeyPressCallBack(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    switch (key)
    {
        case GLFW_KEY_Q:
            glfwSetWindowShouldClose(window, true);
            break;
        default:
            break;
    }
}

void appWindowSizeCallBack(GLFWwindow* window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
}