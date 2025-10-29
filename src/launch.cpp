#include <goli.hpp>
#include "gifenc.h"

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