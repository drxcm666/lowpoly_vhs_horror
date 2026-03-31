#pragma once

#include <raylib.h>

class Player
{
private:
    Camera3D camera_;
    Vector3 position_;
    float eyeHeight_;
    float yaw_;
    float pitch_;
    const float maxPitch_;
    float lookSensitivity_;
    float moveSpeed_;
    float radius_;

public:
    Player();

    void setPosition(const Vector3 &pos) { position_ = pos; };

    void updateLook();
    Vector2 getIntendedMoveDir();

    void applyMoveX(float dx) { position_.x += dx; };
    void applyMoveZ(float dz) { position_.z += dz; };
    void syncCamera();

    const Vector3 &getPosition() const { return position_; }
    const float getRadius() const { return radius_; }
    const Camera3D &getCamera() const { return camera_; }
    const float getMoveSpeed() const { return moveSpeed_; }

    Vector3 getForwardVector() const;
};