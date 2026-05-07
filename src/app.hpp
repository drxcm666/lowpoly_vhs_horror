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
    bool forceExit{false};

public:
    App();
    ~App();
    void run();
};