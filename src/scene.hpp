#pragma once

#include "level_types.hpp"
#include "audio.hpp"

#include "rlights.h"

#include <vector>

struct ModelInstance
{
    std::string name;
    Model model;
    Vector3 position;
    Vector3 rotationAxis;
    float rotationAngle;
    Vector3 scale;
    Color tint;
    bool Culling{false};
    bool positionLock{true};
    Vector3 basePosition;
    float baseRotationAngle;
    Color baseTint;
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
    std::vector<Color> baseLightColors_;
    Shader lightShader_;

    float lookTimer_{0.0f};
    int viewCounts_{0};
    bool isLookingAtMonsterLastFrame_{false};
    bool jumpscarePlayedOnce_{false};

    bool gameOver_{false};

public:
    Scene() = default;
    ~Scene();

    void renderWorld(const Camera3D &camera, bool showDebug) const;
    void loadEnvironment();
    void resetEnvironment(AudioManager &audioManager_);
    void parseCollision(const std::string &path);
    bool checkCollision(const Vector3 &playerPos, float playerRadius) const;
    bool updateEnvironment(
        int currentLoop, const Vector3 &playerPos,
        float playerRadius, AudioManager &audioManager_,
        bool isLookingAtMonster, float dt);
    void parseLightening(const std::string &path);
    void resetInteractables();
    void applyMutations(int currentLoop, AudioManager &audioManager_);

    std::vector<TriggerZone> &getTriggers() { return triggers_; }
    std::vector<Interactable> &getInteractables() { return interactables_; }
    const std::vector<CollisionBlock> &getCollisions() const { return collisionBlocks_; }
};
