cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
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
    src/scene.cpp
    src/player.cpp
    src/audio.cpp
)


target_link_libraries(MyGame PRIVATE 
    raylib
)

target_include_directories(MyGame PRIVATE
    "${CMAKE_SOURCE_DIR}/assets/shaders"
)