#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

constexpr int gWidth{1200};
constexpr int gHeight{800};

void funWindowSizeCallBack(GLFWwindow *window, int width, int height)
{
    glfwSetWindowSize(window, width, height);
}
void funKeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods);

float quadVerticesPos[] = {
    0.0, 0.0, 0.0,
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 1.0, 0.0,
    1.0, 1.0, 0.0,
    1.0, 0.0, 0.0};

float quadPos[] = {
    0.0,
    0.0,
    0.0,
    1.0,
    0.0,
    0.0,
    2.0,
    0.0,
    0.0,
};

float quadCol[] = {
    1.0, 0.0, 0.0,
    0.0, 1.0, 0.0,
    0.0, 0.0, 1.0};

const char *vShaderSource = "#version 330 core\n"
                            "layout (location = 0) in vec3 pos;\n"
                            "uniform mat4 model;\n"
                            "uniform mat4 view;\n"
                            "void main() {\n"
                            "   gl_Position = view * model * vec4(pos, 1.0f);\n"
                            "}\0";

const char *fShaderSource = "#version 330 core\n"
                            "out vec4 FragColor;\n"
                            "uniform vec3 InColor;\n"
                            "void main() {\n"
                            "   FragColor = vec4(InColor, 1.0f);\n"
                            "}\0";

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    GLFWwindow *window = glfwCreateWindow(gWidth, gHeight, "quad experiment", NULL, NULL);
    glfwSetWindowSizeCallback(window, funWindowSizeCallBack);
    glfwMakeContextCurrent(window);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD" << std::endl;
        return -1;
    }

    // Setup Dear ImGui context
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;  // Enable Gamepad Controls
    // io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;         // IF using Docking Branch

    // Setup Platform/Renderer backends
    ImGui_ImplGlfw_InitForOpenGL(window, true); // Second param install_callback=true will install GLFW callbacks and chain to existing ones.
    ImGui_ImplOpenGL3_Init();

    // boolean to control whether the GUI bar should be open
    bool barOpen = true;
    glm::vec3 translate{-1.0, -1.0, 0.0};

    glViewport(0, 0, gWidth, gHeight);

    GLuint VAO[10], VBO;
    unsigned int numQuads = sizeof(quadPos) / (3 * sizeof(float));
    float scale = 1.0 / (float)numQuads;
    std::cout << numQuads << std::endl;
    glGenVertexArrays(numQuads, VAO);
    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVerticesPos), quadVerticesPos, GL_STATIC_DRAW);

    for (unsigned int i = 0; i < numQuads; i++)
    {
        glBindVertexArray(VAO[i]);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void *)0);
        glEnableVertexAttribArray(0);
    }

    GLuint vShader = glCreateShader(GL_VERTEX_SHADER);
    GLuint fShader = glCreateShader(GL_FRAGMENT_SHADER);
    int success;
    glShaderSource(vShader, 1, &vShaderSource, NULL);
    glCompileShader(vShader);
    if (glGetShaderiv(vShader, GL_COMPILE_STATUS, &success); !success)
    {
        std::cout << "vShader compile failed" << std::endl;
    }
    glShaderSource(fShader, 1, &fShaderSource, NULL);
    glCompileShader(fShader);
    if (glGetShaderiv(fShader, GL_COMPILE_STATUS, &success); !success)
    {
        std::cout << "fShader compile failed" << std::endl;
    }
    GLuint programID = glCreateProgram();
    glAttachShader(programID, vShader);
    glAttachShader(programID, fShader);
    glLinkProgram(programID);
    if (glGetProgramiv(programID, GL_LINK_STATUS, &success); !success)
    {
        std::cout << "Program link failed" << std::endl;
    }
    glDeleteShader(vShader);
    glDeleteShader(fShader);

    // pre-render
    glfwSetKeyCallback(window, funKeyCallBack);

    while (!glfwWindowShouldClose(window))
    {
        glfwPollEvents();
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS)
        {
            if (barOpen)
                barOpen = false;
            else
                barOpen = true;
        }
        // Start the Dear ImGui frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        // ImGui::ShowDemoWindow(); // Show demo window! :)

        ImVec2 window_size(800, 600);
        ImGui::SetNextWindowSize(window_size, ImGuiCond_Always);
        if (barOpen)
        {
            ImGui::Begin("quad display interface", &barOpen, ImGuiWindowFlags_MenuBar);
            ImGui::ColorEdit3("Color for left quad", &quadCol[0]);
            ImGui::ColorEdit3("Color for center quad", &quadCol[3]);
            ImGui::ColorEdit3("Color for right quad", &quadCol[6]);

            ImGui::SliderFloat3("translate", glm::value_ptr(translate), -2.0f, 2.0f);
            ImGui::InputFloat3("quad 1 pos", &quadPos[0]);
            ImGui::InputFloat3("quad 2 pos", &quadPos[3]);
            ImGui::InputFloat3("quad 3 pos", &quadPos[6]);

            ImGui::Text("sample text...");
            ImGui::MenuItem("close", "m", &barOpen);
            ImGui::End();
        }

        glClearColor(0.1, 0.1, 0.1, 1.0);
        glClear(GL_COLOR_BUFFER_BIT);

        GLint loc;
        for (unsigned int i = 0; i < numQuads; i++)
        {
            glUseProgram(programID);
            glBindVertexArray(VAO[i]);
            loc = glGetUniformLocation(programID, "InColor");
            glUniform3f(loc, quadCol[3 * i + 0], quadCol[3 * i + 1], quadCol[3 * i + 2]);
            loc = glGetUniformLocation(programID, "model");
            glm::mat4 model{1.0f};
            model = glm::translate(model, glm::vec3(quadPos[3 * i + 0], quadPos[3 * i + 1], quadPos[3 * i + 2]));
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(model));
            loc = glGetUniformLocation(programID, "view");
            glm::mat4 view{1.0f};
            view = glm::translate(view, translate);
            view = glm::scale(view, glm::vec3(2.0 * scale, 2.0 * 1.0, 1.0));
            glUniformMatrix4fv(loc, 1, GL_FALSE, glm::value_ptr(view));
            glDrawArrays(GL_TRIANGLES, 0, 6);
        }

        // Rendering
        // (Your code clears your framebuffer, renders your other stuff etc.)
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        // (Your code calls glfwSwapBuffers() etc.)

        glfwSwapBuffers(window);
    }

    glDeleteBuffers(1, &VBO);
    glDeleteVertexArrays(numQuads, VAO);
    glfwTerminate();

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}

void funKeyCallBack(GLFWwindow *window, int key, int scancode, int action, int mods)
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