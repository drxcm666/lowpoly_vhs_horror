#pragma once

#include "scene.hpp"
#include "player.hpp"
#include "audio.hpp"
#include "dialogue_manager.hpp"

#include <raylib.h>

#include <string>
#include <vector>

enum class GameState
{
    START,
    DIALOGUE,
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
    DialogueManager dialogue_;
    std::string currentPrompt_;
    std::string objectiveText_;
    float stepLength_;
    float distanceAccumulator_;
    int loopIteration_;

    bool insideStore_{false};
    bool isDebugMode_{true};
    bool noclipEnabled_{false};
    Font myFont_;

    float getInteractionScore(const Vector3 &targetPos, float maxDistance) const;
    void updatePlayerMovement(float dt);
    void updateTriggers();
    void findFocusedTarget(int &outInteractableIndex, int &outDoorIndex);
    void handleInteraction(int interactableIndex, int doorIndex);
    void onZoneChanged();

public:
    Game();
    void update(float dt);
    void renderWorld() const;
    void renderHud(int screenWidth, int screenHeight);
    // void render(int screenWidth, int screenHeight) const;
    bool getDebugMode() { return isDebugMode_; }
    void toggleDebugMode() { isDebugMode_ = !isDebugMode_; }
};