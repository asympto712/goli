# Project structure (updated)
GOAL: GUI app where users can view, edit, and explore the various configurations of Conway's Game Of Life
Tech stack: C++. openGL, Dear Imgui

# Core functionalities
Mode changes among Start, View, Focus, and Edit
Responsibilities:
- View mode
    - Replay
    - speed up / down
    - record (.gif)
    - select a chunk and go to Edit mode
    - go to Focus mode
    - save the current states
- Focus mode
    - minimal interface
    - player can fly through the 3D space and land on the grid
    - go back to view mode
- Edit mode
    - click to toggle cell states
    - clear the selected chunk
    - extend / shrink the editted range (Users can choose whether to use the states from view mode, or ignore them)
    - preview to observe the time evolution of the editted chunk
    - save the config to presets
    - overwrite the edit region with a preset
    - update the preset
    - go back to view mode
- Start mode
    - first UI the user will see
    - select from preset to see in View mode
    - start with empty grid to the View mode
    - select world size (should be resizable)

# Roadmap
## first phase (only OpenGL)
### step1: draw simple quads on OpenGL - done
### step2: implement game - done
TMP: one-step in GoL: state is managed by 2-dim array of one unsigned int
-> Change: now the state is directly linked the tile color, 4 byte struct that represents RGBA values
### step3: test texture buffer object for writing/reading the cell states to render - done
### step4: implement camera, view change system - done
Game should have two modes, _view_ and _land_. _View_ is static, looking at the whole picture, _land_ is where you land on the picture and walk around
- Really want this animation where the transition from _view_ mode to _land_ mode is smooth
- one idea: setup **animation** switch, which prohibits key inputs and "play" the animation (flying according to precalculated trajectory) while it is true. 
### step5: integrate camera into GoL - done
game loop and render loop should operate on different threads, when one step in game done -> signal the render loop that the new texture buffer is ready
### step5.5 (side quest) : investigate the movement speed issue (the velocity doesn't match the reality) - done
### step6 : Add edit mode

## second phase (refactor, introduction of C++ 20 features)
### better abstraction of App class
### bitfield representation of states
### more efficient render pipeline

## third phase (NOW)
### Get used to Imgui library
edited `quad.cpp` to integrate ImGui (Also fixed the position of `glDrawArray`)
### integrate ViewApp with Imgui