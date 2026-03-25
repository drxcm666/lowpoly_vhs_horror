cmake_minimum_required(VERSION 3.25)
project(vhs_horror LANGUAGES CXX)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)
FetchContent_MakeAvailable(raylib)

add_executable(MyGame
    src/main.cpp
    src/app.cpp
    src/game.cpp
)


target_link_libraries(MyGame PRIVATE 
    raylib
)