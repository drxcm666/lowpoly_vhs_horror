#include "game.hpp"
#include "scene.hpp"

#include <cmath>
#include <cstddef>

Game::Game() : state_{GameState::START}, objectiveText_{"Go inside the gas station"}
{
    Vector3 startPos = player_.getPosition();
    scene_.parseCollision("assets/levels/room01_collision.txt");
    scene_.loadEnvironment();
    player_.setPosition(startPos);
    player_.syncCamera();
    audioManager_.loadFromManifest("assets/music/audio_manifest.txt");
    audioManager_.parseEmitters("assets/levels/emitters.txt");
}

float Game::getInteractionScore(const Vector3 &targetPos, float maxDistance) const
{
    float distance = std::sqrt(
        (player_.getCamera().position.x - targetPos.x) * (player_.getCamera().position.x - targetPos.x) +
        (player_.getCamera().position.z - targetPos.z) * (player_.getCamera().position.z - targetPos.z) +
        (player_.getCamera().position.y - targetPos.y) * (player_.getCamera().position.y - targetPos.y));

    if (distance < maxDistance && distance > 0.001f)
    {
        float targetX = (targetPos.x - player_.getCamera().position.x) / distance;
        float targetZ = (targetPos.z - player_.getCamera().position.z) / distance;
        float targetY = (targetPos.y - player_.getCamera().position.y) / distance;

        float forwardX = player_.getForwardVector().x;
        float forwardZ = player_.getForwardVector().z;
        float forwardY = player_.getForwardVector().y;

        return (targetX * forwardX + targetZ * forwardZ + targetY * forwardY);
    }

    return -1.0f;
}

void Game::updatePlayerMovement(float dt)
{
    Vector2 wishDir = player_.getIntendedMoveDir();

    float stepX = wishDir.x * player_.getMoveSpeed() * dt;
    float stepZ = wishDir.y * player_.getMoveSpeed() * dt;

    if (noclipEnabled_)
    {
        player_.applyMoveZ(stepZ);
        player_.applyMoveX(stepX);
        return;
    }

    Vector3 currentPos = player_.getPosition();
    float radius = player_.getRadius();

    Vector3 testPosZ = {currentPos.x, currentPos.y, currentPos.z + stepZ};
    if (!scene_.checkCollision(testPosZ, radius))
        player_.applyMoveZ(stepZ);

    Vector3 testPosX = {currentPos.x + stepX, currentPos.y, currentPos.z};
    if (!scene_.checkCollision(testPosX, radius))
        player_.applyMoveX(stepX);
}

void Game::updateTriggers()
{
    for (auto &trigger : scene_.getTriggers())
    {
        float cornerX = trigger.position.x - trigger.size.x / 2.0f;
        float cornerZ = trigger.position.z - trigger.size.z / 2.0f;

        Rectangle triggerBox = {cornerX, cornerZ, trigger.size.x, trigger.size.z};
        bool isInsideNow = CheckCollisionCircleRec(
            {player_.getPosition().x, player_.getPosition().z}, player_.getRadius(), triggerBox);

        if (isInsideNow && !trigger.wasInsideLastFrame &&
            (trigger.type == TriggerType::FrontDoors || trigger.type == TriggerType::BackDoors))
        {
            insideStore = !insideStore;

            if (insideStore && trigger.type == TriggerType::FrontDoors)
            {
                audioManager_.playSound("sfx_bell");
            }
        }

        if (isInsideNow && !trigger.wasInsideLastFrame)
        {
            if (trigger.type == TriggerType::Noise && state_ == GameState::NOISE)
            {
                objectiveText_ = "RUNNNNN";
            }
            // else if (trigger.type == TriggerType::FrontDoors || trigger.type == TriggerType::BackDoors)
            // {
            //     audioManager_.playMusic("mus_background");
            //     audioManager_.playMusic("mus_cricket");
            // }

            trigger.active = false;
        }

        if (!isInsideNow)
        {
            trigger.wasInsideLastFrame = false;
            trigger.active = true;
        }
        else
            trigger.wasInsideLastFrame = true;
    }

    if (insideStore)
    {
        audioManager_.playEmitter("mus_refrigerator");
        audioManager_.playEmitter("mus_radio");
        audioManager_.playEmitter("mus_store_lamp");
        audioManager_.playEmitter("mus_coffee_machine");
        
        audioManager_.stopMusic("mus_store_sign");
        audioManager_.stopMusic("mus_background");
        audioManager_.stopMusic("mus_crickets");
    }
    else
    {
        audioManager_.playEmitter("mus_store_sign");
        audioManager_.playEmitter("mus_test");
        audioManager_.playMusic("mus_background");
        audioManager_.playMusic("mus_cricket");

        audioManager_.stopMusic("mus_refrigerator");
        audioManager_.stopMusic("mus_radio");
        audioManager_.stopMusic("mus_store_lamp");
        audioManager_.stopMusic("mus_coffee_machine");
    }
}

void Game::findFocusedTarget(int &outInteractableIndex, int &outDoorIndex)
{
    float bestScore = -1.0f;

    auto &interactables = scene_.getInteractables();
    for (std::size_t i = 0; i < interactables.size(); i++)
    {
        if (interactables[i].active)
        {
            float score = getInteractionScore(interactables[i].position, 2.0f);

            if (score > 0.9f && score > bestScore)
            {
                bestScore = score;
                outInteractableIndex = i;
                outDoorIndex = -1;
            }
        }
    }
}

void Game::handleInteraction(int interactableIndex, int doorIndex)
{
    if (interactableIndex != -1)
    {
        currentPrompt_ = scene_.getInteractables()[interactableIndex].promptText;

        if (IsKeyPressed(KEY_E) &&
            (scene_.getInteractables()[interactableIndex].type == InteractiveType::Worker))
        {
            scene_.getInteractables()[interactableIndex].active = false;
            state_ = GameState::NOISE;
            objectiveText_ = "Check for strange noises near fuel pumps";
        }
    }
}

void Game::update(float dt)
{
    game_dt = dt;

    currentPrompt_ = "";

    if (IsKeyPressed(KEY_K))
        noclipEnabled_ = !noclipEnabled_;

    if (IsKeyPressed(KEY_F3))
        Game::toggleDebugMode();

    player_.updateLook();

    updatePlayerMovement(dt);

    player_.syncCamera();

    updateTriggers();

    int targetInteractableIndex = -1;
    int targetDoorIndex = -1;
    findFocusedTarget(targetInteractableIndex, targetDoorIndex);

    handleInteraction(targetInteractableIndex, targetDoorIndex);

    audioManager_.setListenerPosition(player_.getCamera().position);
    audioManager_.update(dt);
}

void Game::render() const
{
    scene_.renderWorld(player_.getCamera(), isDebugMode_);

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawCircle(screenWidth / 2, screenHeight / 2, 3.0f, Fade(RAYWHITE, 0.5f));

    if (!currentPrompt_.empty())
    {
        int textWidth = MeasureText(currentPrompt_.c_str(), 20);
        DrawText(currentPrompt_.c_str(), (screenWidth - textWidth) / 2, screenHeight / 2 - 40, 20, RAYWHITE);
    }
    if (!objectiveText_.empty())
    {
        DrawText(objectiveText_.c_str(), 30, 30, 20, GOLD);
    }
}
