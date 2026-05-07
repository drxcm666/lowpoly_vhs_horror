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
    NORMAL,
    DIALOGUE,
    END
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
    bool isDebugMode_{false};
    bool noclipEnabled_{false};
    bool isLookingAtMonster_{false};
    Font myFont_;

    float getInteractionScore(const Vector3 &targetPos, float maxDistance) const;
    void updatePlayerMovement(float dt);
    void updateTriggers();
    void findFocusedTarget(int &outInteractableIndex);
    void handleInteraction(int interactableIndex);
    void onZoneChanged();
    void handleGameEvent(const std::string &eventName);

public:
    Game();
    void update(float dt);
    void renderWorld() const;
    void renderHud(int screenWidth, int screenHeight);
    bool getDebugMode() { return isDebugMode_; }
    GameState getGameState() { return state_; }
    void toggleDebugMode() { isDebugMode_ = !isDebugMode_; }
};