#include "game.hpp"
#include <cmath>

Game::Game() : camera_{}, playerPosition_{0.0f, 0.0f, 0.0f}, eyeHeight_{1.7f},
               yaw_{0.0f}, pitch_{0.0f}, maxPitch_{1.5f},
               lookSensitivity_{0.003f}, moveSpeed_{5.0f}
{
    syncCameraWithPlayer();
    camera_.up = {0.0f, 1.0f, 0.0f};
    camera_.fovy = 60.0f;
    camera_.projection = CAMERA_PERSPECTIVE;
    walls_.push_back({{10.0f, 2.0f, 0.0f}, {1.0f, 4.0f, 20.0f}});
    walls_.push_back({{-10.0f, 2.0f, 0.0f}, {1.0f, 4.0f, 20.0f}});
    walls_.push_back({{0.0f, 2.0f, 10.0f}, {20.0f, 4.0f, 1.0f}});
    walls_.push_back({{0.0f, 2.0f, -10.0f}, {20.0f, 4.0f, 1.0f}});
}

void Game::update(float dt)
{
    Vector2 mouseDelta = GetMouseDelta();
    yaw_ -= (mouseDelta.x) * lookSensitivity_;
    pitch_ -= (mouseDelta.y) * lookSensitivity_;
    float forwardX = sin(yaw_);
    float forwardZ = cos(yaw_);
    float rightX = sin(yaw_ - PI / 2);
    float rightZ = cos(yaw_ - PI / 2);
    float moveDirX{0.0f};
    float moveDirZ{0.0f};

    if (pitch_ > maxPitch_)
        pitch_ = maxPitch_;
    if (pitch_ < -maxPitch_)
        pitch_ = -maxPitch_;

    if (IsKeyDown(KEY_W))
    {
        moveDirZ += forwardZ;
        moveDirX += forwardX;
    }
    if (IsKeyDown(KEY_S))
    {
        moveDirZ -= forwardZ;
        moveDirX -= forwardX;
    }
    if (IsKeyDown(KEY_D))
    {
        moveDirZ += rightZ;
        moveDirX += rightX;
    }
    if (IsKeyDown(KEY_A))
    {
        moveDirZ -= rightZ;
        moveDirX -= rightX;
    }

    float length = std::sqrt(moveDirZ * moveDirZ + moveDirX * moveDirX);
    if (length > 0.0f)
    {
        moveDirZ /= length;
        moveDirX /= length;
    }

    playerPosition_.z += moveDirZ * moveSpeed_ * dt;
    playerPosition_.x += moveDirX * moveSpeed_ * dt;

    syncCameraWithPlayer();
}

void Game::render() const
{
    BeginMode3D(camera_);

    DrawCube(Vector3{0.0f, -0.5f, 0.0f}, 20.0f, 1.0f, 20.0f, LIGHTGRAY);
    for (const auto &i : walls_)
    {
        DrawCube(i.position, i.size.x, i.size.y, i.size.z, GRAY);
    }

    EndMode3D();
}

void Game::syncCameraWithPlayer()
{
    Vector3 eyePosition{playerPosition_.x,
                        playerPosition_.y + eyeHeight_,
                        playerPosition_.z};

    camera_.position = eyePosition;

    Vector3 direction{cos(pitch_) * sin(yaw_),
                      sin(pitch_),
                      cos(pitch_) * cos(yaw_)};

    camera_.target = {eyePosition.x + direction.x,
                      eyePosition.y + direction.y,
                      eyePosition.z + direction.z};
}