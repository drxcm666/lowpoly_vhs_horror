#pragma once

#include "scene.hpp"
#include "player.hpp"

#include <raylib.h>

#include <string>
#include <vector>

enum class GameState
{
    START,
    FIRST_CONVERSATION,
    NOISE,
    ESCAPED
};

class Game
{
private:
    Scene scene_;
    Player player_;
    GameState state_;
    std::string currentPrompt_;
    std::string objectiveText_;

    static bool isDebugMode_;

    float getInteractionScore(const Vector3 &targetPos, float maxDistance) const;
    void updatePlayerMovement(float dt);
    void updateTriggers();
    void findFocusedTarget(int &outInteractableIndex, int &outDoorIndex);
    void handleInteraction(int interactableIndex, int doorIndex);

public:
    Game();
    void update(float dt);
    void render() const;
    static bool getDebugMode() { return isDebugMode_; }
    static void toggleDebugMode() { isDebugMode_ = !isDebugMode_; }
};