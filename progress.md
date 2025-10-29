# Project structure (tmp)
- implementation of Conway's Game Of Life
- handling the OpenGL side (render, shader, etc..)
- Connecting the two (passing the cell state as a drawable object)

# Roadmap
## step1: draw simple quads on OpenGL - done
## step2: implement game - done
TMP: one-step in GoL: state is managed by 2-dim array of one unsigned int
-> Change: now the state is directly linked the tile color, 4 byte struct that represents RGBA values
## step3: test texture buffer object for writing/reading the cell states to render - done
## step4: implement camera, view change system - done
Game should have two modes, _view_ and _land_. _View_ is static, looking at the whole picture, _land_ is where you land on the picture and walk around
- Really want this animation where the transition from _view_ mode to _land_ mode is smooth
- one idea: setup **animation** switch, which prohibits key inputs and "play" the animation (flying according to precalculated trajectory) while it is true. 
## step5: integrate camera into GoL - done
game loop and render loop should operate on different threads, when one step in game done -> signal the render loop that the new texture buffer is ready
## step5.5 (side quest) : investigate the movement speed issue (the velocity doesn't match the reality) - done
## step6 : Add edit mode