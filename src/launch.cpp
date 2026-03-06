#include "goli.hpp"
#include "gifenc.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"

int main(int argc, char* argv[])
{
    if (argc == 1)
    {
        launchRecSession();
    }
    else if (argc == 2)
    {
        if (strcmp(argv[1], "--gif") != 0)
        {
            printf("Unknown option. Accepted options are: --gif (record the window to .gif file)\n");
            exit(1);
        }
        else
        {
            launchRecSession();
        }
    }
}