#pragma once

#include <raylib.h>

#include <vector>

struct Wall
{
    Vector3 position;
    Vector3 size;
};


class Game
{
private:
    Camera3D camera_;
    Vector3 playerPosition_;
    float eyeHeight_;
    float yaw_;
    float pitch_;
    const float maxPitch_;
    float lookSensitivity_;
    float moveSpeed_;
    std::vector<Wall> walls_;

    void syncCameraWithPlayer();

public:
    Game();
    void update(float dt);
    void render() const;
};