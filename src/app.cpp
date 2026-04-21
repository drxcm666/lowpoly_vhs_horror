#include "app.hpp"

App::App(int screenWidth, int screenHeight)
{
    InitWindow(screenWidth, screenHeight, "Game");
    InitAudioDevice();
    DisableCursor();
    SetTargetFPS(60);

    renderWidth_ = screenWidth / 3;
    renderHeight_ = screenHeight / 3;
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
    while (!WindowShouldClose())
    {
        float dt = GetFrameTime();

        game_->update(dt);

        BeginTextureMode(renderTarget_);
        ClearBackground(Color{1, 1, 3, 255});
        game_->renderWorld();
        // game_->renderHud(renderWidth_, renderHeight_);
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