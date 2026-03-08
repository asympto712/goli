#pragma once

#include "app_interface.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "gui_app_config.hpp"

// abstract class that should provide framework-agnostic GUI interface
class GuiInterface
{
    public:
    virtual BasicApp& mutable_app() = 0;
    virtual const BasicApp& c_app() const = 0;
    virtual void contextWindow(GLFWwindow* window) = 0; // contexualize the window
    virtual void newFrame() = 0;  // prepare to step into the new frame (poll inputs, calculate deltas, etc..)
    virtual void drawWidget() = 0; // call to draw the desired widget
    virtual void initGui() = 0;
    virtual void render() = 0; // render call intended to be used right before glfwSwapBuffers(window)
    virtual void handleInput() = 0;
};

// GUI class adapted to Dear ImGUI that handles the boilerplates
class DearImGui: public GuiInterface
{
    public:

    // mode agnostic functions (boilerplates)
    DearImGui() = delete;
    DearImGui(BasicApp& app);
    BasicApp& mutable_app() override;
    const BasicApp& c_app() const override;
    void initGui() override;
    void contextWindow(GLFWwindow* window) override;
    void newFrame() override; // should be called at the beginning of every game loop, before calling handleInput()
    void render() override;
    bool dispatchMouseInput(); // returns true if the mouse input should be dispatched to the underlying app as well
    bool dispatchKeyInput(); // returns true if the key state should be dispatched to the underlying app as well

    // mode specific functions
    // void drawWidget() override;
    // void handleInput() override;

    private:
    BasicApp& m_app;
};

DearImGui::DearImGui(BasicApp& app):
m_app{app} {}

BasicApp& DearImGui::mutable_app()
{
    return m_app;
}
const BasicApp& DearImGui::c_app() const
{
    return m_app;
}
void DearImGui::initGui()
{
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    contextWindow(c_app().c_window());
    ImGui_ImplOpenGL3_Init();
}
void DearImGui::contextWindow(GLFWwindow* window)
{
    ImGui_ImplGlfw_InitForOpenGL(window, true);
}
void DearImGui::newFrame()
{
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}
bool DearImGui::dispatchMouseInput()
{
    ImGuiIO& io = ImGui::GetIO();
    return !io.WantCaptureMouse;
}
bool DearImGui::dispatchKeyInput()
{
    ImGuiIO& io = ImGui::GetIO();
    return !io.WantCaptureKeyboard;
}
void DearImGui::render()
{
    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

// GUI for View mode
class ViewModeGui: DearImGui
{
    public:
    // mode specific functions
    void drawWidget() override;
    void handleInput() override;
};

void ViewModeGui::drawWidget()
{
    ImVec2 widgetSize{VIEWMODE_PANEL_W, VIEWMODE_WINDOW_H};
    ImVec2 widgetPos{VIEWMODE_WINDOW_W, 0};
    ImGui::SetNextWindowSize(widgetSize, ImGuiCond_Always);
    ImGui::SetNextWindowPos(widgetPos, ImGuiCond_Always);
    ImGui::Begin("ViewMode");

    // TODO!

    ImGui::End();
}
void ViewModeGui::handleInput()
{
    // TODO!
}


