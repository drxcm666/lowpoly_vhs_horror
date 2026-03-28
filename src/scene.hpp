#pragma once

#include "level_types.hpp"

#include <vector>

class Scene
{
private:
    std::vector<Wall> walls_;
    std::vector<Door> doors_;
    std::vector<TriggerZone> triggers_;
    std::vector<Interactable> interactables_;

    const float tileSize_{4.0f};
    int levelWidthTiles_{0};
    int levelHeightTiles_{0};

    Model gasModel_;
    Model gasGlassModel_;
    Model gasBushesModel_;
    Model gasBushesModel0_;
    Model gasBushesModel1_;
    Model gasBushesModel2_;
    Model gasBushesModel3_;
    Model gasBushesModel4_;

    // Shader pipeline is temporarily disabled.
    // Shader lightingShader_;
    // int lightPosLoc_;
    // int lightColorLoc_;
    // int viewPosLoc_;
    // Vector3 lampPosition_;

    Vector3 modelPosition_{20.0f, 0.0f, 20.0f};
    Vector3 modelScale_;
    float modelRotationAngle_;

    bool checkCollisionSingleWall(const Vector3 &playerPos, const Wall &wall, float playerRadius) const;

public:
    bool isPowerOn_{true};

    Scene() = default;
    ~Scene();

    void loadLevelFromTextFile(const std::string &path, Vector3 &playerPosition);
    bool checkCollisionAllWalls(const Vector3 &playerPos, float playerRadius) const;
    void renderWorld(Camera3D camera) const;

    std::vector<TriggerZone> &getTriggers() { return triggers_; }
    std::vector<Interactable> &getInteractables() { return interactables_; }
    std::vector<Door> &getDoors() { return doors_; }
};

