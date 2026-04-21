#pragma once

#include "level_types.hpp"
#include "audio.hpp"

#include "rlights.h"

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
    bool positionLock{true};
};

class Scene
{
private:
    std::vector<CollisionBlock> collisionBlocks_;
    std::vector<Door> doors_;
    std::vector<TriggerZone> triggers_;
    std::vector<Interactable> interactables_;

    std::vector<ModelInstance> models_;
    std::vector<LightingFixtures> lightSourcesTxt_;
    std::vector<Light> lightSources_;
    Shader lightShader_;

public:
    Scene() = default;
    ~Scene();

    void renderWorld(const Camera3D &camera, bool showDebug) const;
    void loadEnvironment();
    void parseCollision(const std::string &path);
    bool checkCollision(const Vector3 &playerPos, float playerRadius) const;
    void parseLightening(const std::string &path);
    void resetInteractables();
    void applyMutations(int currentLoop);

    std::vector<TriggerZone> &getTriggers() { return triggers_; }
    std::vector<Interactable> &getInteractables() { return interactables_; }
    std::vector<Door> &getDoors() { return doors_; }
};
