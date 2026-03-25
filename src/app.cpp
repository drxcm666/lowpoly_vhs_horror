#include "app.hpp"

App::App(int screenWidth, int screenHeight)
{
    InitWindow(screenWidth, screenHeight, "Game");
    DisableCursor();
    SetTargetFPS(60);

    game_ = std::make_unique<Game>();
}

App::~App()
{
    CloseWindow();
}

void App::run()
{
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        game_->update(dt);

        BeginDrawing();
        ClearBackground(RAYWHITE);
        
        game_->render();

        EndDrawing();
    }
}