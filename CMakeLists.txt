cmake_minimum_required(VERSION 3.14.0)

project(GoLI VERSION 0.1)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

find_package(glfw3 3.3 REQUIRED)
find_package(OpenGL REQUIRED)
find_package(glm REQUIRED)

add_executable(quad extern/src/glad.c src/quad.cpp)

target_include_directories(quad PRIVATE include extern/include ${glfw3_INCLUDE_DIRS})
target_link_libraries(quad PRIVATE glfw OpenGL::GL glm::glm-header-only)

set_target_properties(quad PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY out/debug
)

add_executable(game src/game.cpp)

target_include_directories(game PRIVATE include)
set_target_properties(game PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY out/debug
)

add_executable(app_demo extern/src/glad.c src/game.cpp src/app.cpp src/app_demo.cpp)
target_include_directories(app_demo PRIVATE include extern/include ${glfw3_INCLUDE_DIRS})
target_link_libraries(app_demo PRIVATE glfw OpenGL::GL glm::glm-header-only)
set_target_properties(app_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY out/debug
)

add_executable(camera_demo extern/src/glad.c src/game.cpp src/camera_demo.cpp)
target_include_directories(camera_demo PRIVATE include extern/include ${glfw3_INCLUDE_DIRS})
target_link_libraries(camera_demo PRIVATE glfw OpenGL::GL glm::glm-header-only)
set_target_properties(camera_demo PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY out/debug
)

add_executable(launch extern/src/glad.c extern/src/gifenc.c src/game.cpp src/camera.cpp src/app.cpp src/goli.cpp src/launch.cpp)
target_include_directories(launch PRIVATE include extern/include extern/include/gifenc ${glfw3_INCLUDE_DIRS})
target_link_libraries(launch PRIVATE glfw OpenGL::GL glm::glm-header-only)
set_target_properties(launch PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY out/debug
)