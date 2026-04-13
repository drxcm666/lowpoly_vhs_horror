#pragma once

#include "game.hpp"

#include <memory>

class App
{
private:
    std::unique_ptr<Game> game_;
    RenderTexture2D renderTarget_{};
    int renderWidth_{0};
    int renderHeight_{0};

public:
    App(int screenWidth = 1280, int screenHeight = 960);
    ~App();
    void run();
};