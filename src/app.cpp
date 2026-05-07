#include "app.hpp"

App::App()
{
    SetConfigFlags(FLAG_FULLSCREEN_MODE);

    InitWindow(0, 0, "Game");

    int monitor = GetCurrentMonitor();
    SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
    SetWindowPosition(0, 0);

    InitAudioDevice();
    DisableCursor();
    SetTargetFPS(60);

    renderWidth_ = GetMonitorWidth(monitor) / 3;
    renderHeight_ = GetMonitorHeight(monitor) / 3;
    renderTarget_ = LoadRenderTexture(renderWidth_, renderHeight_);
    SetTextureFilter(renderTarget_.texture, TEXTURE_FILTER_BILINEAR);
    
    game_ = std::make_unique<Game>();
}

App::~App()
{
    UnloadRenderTexture(renderTarget_);
    CloseAudioDevice();
    CloseWindow();
}

void App::run()
{
    auto state = game_->getGameState();
    while (!WindowShouldClose() && !forceExit)
    {
        if (game_->getGameState() == GameState::END)
            forceExit = true;

        float dt = GetFrameTime();

        game_->update(dt);

        BeginTextureMode(renderTarget_);
        ClearBackground(Color{1, 1, 3, 255});
        game_->renderWorld();
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle source = {0.0f, 0.0f, 
            (float)renderTarget_.texture.width, 
            -(float)renderTarget_.texture.height};
        Rectangle destination = {0.0f, 0.0f, 
            (float)GetScreenWidth(), 
            (float)GetScreenHeight()};
        DrawTexturePro(renderTarget_.texture, source, destination, {0.0f, 0.0f}, 0.0f, WHITE);

        game_->renderHud(GetScreenWidth(), GetScreenHeight());

        EndDrawing();
    }
}