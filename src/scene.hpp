#pragma once

#include "level_types.hpp"

#include <vector>

struct ModelInstance
{
    Model model;
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    Vector3 scale;
    Color tint;
    bool Culling{false};
};

class Scene
{
private:
    std::vector<CollisionBlock> collisionBlocks_;
    std::vector<Door> doors_;
    std::vector<TriggerZone> triggers_;
    std::vector<Interactable> interactables_;

    const float tileSize_{4.0f};
    int levelWidthTiles_{0};
    int levelHeightTiles_{0};

    std::vector<ModelInstance> models_;
    // Shader pipeline is temporarily disabled.
    // Shader lightingShader_;
    // int lightPosLoc_;
    // int lightColorLoc_;
    // int viewPosLoc_;
    // Vector3 lampPosition_;

    void parseCollision(const std::string &path);

public:
    Scene() = default;
    ~Scene();

    void renderWorld(const Camera3D &camera, bool showDebug) const;
    void loadEnvironment();
    void loadCollisionFile(const std::string &path);
    bool checkCollision(const Vector3 &playerPos, float playerRadius) const;

    std::vector<TriggerZone> &getTriggers() { return triggers_; }
    std::vector<Interactable> &getInteractables() { return interactables_; }
    std::vector<Door> &getDoors() { return doors_; }
};
