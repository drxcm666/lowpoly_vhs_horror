#pragma once

#include "game.hpp"

#include <memory>

class App
{
private:
    std::unique_ptr<Game> game_;

public:
    App(int screenWidth = 1280, int screenHeight = 960);
    ~App();
    void run();
};