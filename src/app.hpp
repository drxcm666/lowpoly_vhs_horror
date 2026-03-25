#pragma once

#include "game.hpp"

#include <memory>

class App
{
private:
    std::unique_ptr<Game> game_;

public:
    App(int screenWidth = 1000, int screenHeight = 650);
    ~App();
    void run();
};