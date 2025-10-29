#pragma once

#include <app.hpp>
#include <camera.hpp>
#include "gifenc/gifenc.h"

void GAwindowSizeCallback(GLFWwindow* window, int width, int height);

void GAKeyPressCallback(GLFWwindow* window, int key, int scancode, int action, int mods);

void GACursorPosCallback(GLFWwindow* window, double xpos, double ypos);

void GAMouseButtonCallback(GLFWwindow* window, int button, int action, int mods);

class GoliApp: public App
{
    public:
    void preRender() override;
    void render() override;
    void run() override;
    void setCallbacks() override;
    void init();
    Camera cam;
    using App::App;
    GoliApp(ui dimX, ui dimY, int seed):
        cam(),
        paused(false),
        App::App(dimX, dimY, seed)
        {}
    void updateLastTime();
    void runWithRec(ge_GIF* gif);

    private:
    /// @brief is the simulation paused?
    bool paused;
    void processInput();
    /// @brief timer value when the key was pressed last time.
    double lastKeyPress;
    const double keyInputTimeout = 0.5;
};

inline int launch()
{
    GoliApp app(20, 20, 12);
    app.run();
    return 0;
}

inline int launchRecSession()
{
    ge_GIF *gif = ge_new_gif(
        "out/img/demo.gif",
        gWidth,
        gHeight,
        (uint8_t *) NULL,
        8,
        -1,
        0
    );

    for (int i=0; i< gWidth * gHeight; i++)
    {
        if (gif->frame + i == NULL)
        {
            printf("gif->frame[%d] is an invalid memory!", i);
        }
    }
    GoliApp app{20, 20, 7};
    app.runWithRec(gif);

    ge_close_gif(gif);
    return 0;
}