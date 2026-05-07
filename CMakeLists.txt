cmake_minimum_required(VERSION 3.25)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
project(vhs_horror LANGUAGES CXX)

set(CMAKE_POLICY_VERSION_MINIMUM 3.5)

include(FetchContent)
FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.0
)

FetchContent_Declare(
    nlohmann_json
    GIT_REPOSITORY https://github.com/nlohmann/json.git
    GIT_TAG v3.11.2
)

FetchContent_MakeAvailable(raylib nlohmann_json)

add_executable(MyGame
    src/main.cpp
    src/app.cpp
    src/game.cpp
    src/scene.cpp
    src/player.cpp
    src/audio.cpp
    src/dialogue_manager.cpp
    src/download_textures.cpp
)


target_link_libraries(MyGame PRIVATE 
    raylib
    nlohmann_json::nlohmann_json
)

target_include_directories(MyGame PRIVATE
    "${CMAKE_SOURCE_DIR}/assets/shaders"
    "${CMAKE_SOURCE_DIR}/assets/dialogues"
)