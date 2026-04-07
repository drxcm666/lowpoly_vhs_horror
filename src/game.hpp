#pragma once

#include "scene.hpp"
#include "player.hpp"
#include "audio.hpp"

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
    AudioManager audioManager_;
    std::string currentPrompt_;
    std::string objectiveText_;

    float game_dt{};

    bool insideStore{false};

    bool isDebugMode_{true};
    bool noclipEnabled_{false};

    float getInteractionScore(const Vector3 &targetPos, float maxDistance) const;
    void updatePlayerMovement(float dt);
    void updateTriggers();
    void findFocusedTarget(int &outInteractableIndex, int &outDoorIndex);
    void handleInteraction(int interactableIndex, int doorIndex);

public:
    Game();
    void update(float dt);
    void render() const;
    bool getDebugMode() { return isDebugMode_; }
    void toggleDebugMode() { isDebugMode_ = !isDebugMode_; }
};