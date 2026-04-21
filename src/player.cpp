#include "player.hpp"

#include <cmath>

Player::Player() : camera_{}, position_{9.0f, 0.0f, -17.401f}, eyeHeight_{1.7f},
                   yaw_{-PI / 2.0}, pitch_{0.0f}, maxPitch_{1.5f},
                   lookSensitivity_{0.002f}, moveSpeed_{5.0f},
                   radius_{0.4f}
{
    camera_.up = {0.0f, 1.0f, 0.0f};
    camera_.fovy = 60.0f;
    camera_.projection = CAMERA_PERSPECTIVE;
}

void Player::updateLook()
{
    Vector2 mouseDelta = GetMouseDelta();
    yaw_ -= (mouseDelta.x) * lookSensitivity_;
    pitch_ -= (mouseDelta.y) * lookSensitivity_;

    if (pitch_ > maxPitch_)
        pitch_ = maxPitch_;
    if (pitch_ < -maxPitch_)
        pitch_ = -maxPitch_;
}

Vector3 Player::getForwardVector() const
{
    return Vector3{
        static_cast<float>(cos(pitch_) * sin(yaw_)),
        static_cast<float>(sin(pitch_)),
        static_cast<float>(cos(pitch_) * cos(yaw_))};
}

Vector2 Player::getIntendedMoveDir()
{
    // Look vectors
    float forwardX = sin(yaw_);
    float forwardZ = cos(yaw_);

    // Right side vector
    float rightX = sin(yaw_ - PI / 2);
    float rightZ = cos(yaw_ - PI / 2);

    // Direction indicator
    float moveDirX{0.0f};
    float moveDirZ{0.0f};

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

    return Vector2{moveDirX, moveDirZ};
}

void Player::syncCamera()
{
    Vector3 eyePosition{position_.x,
                        position_.y + eyeHeight_,
                        position_.z};

    camera_.position = eyePosition;

    Vector3 direction = getForwardVector();

    camera_.target = {eyePosition.x + direction.x,
                      eyePosition.y + direction.y,
                      eyePosition.z + direction.z};
}