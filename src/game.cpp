#include "game.hpp"
#include "scene.hpp"
#include "dialogue_manager.hpp"

#include <cmath>
#include <cstddef>

Game::Game() : state_{GameState::START}, objectiveText_{"Go inside the gas station"},
               stepLength_{1.8f}, distanceAccumulator_{0.0f}, loopIteration_{0}
{
    myFont_ = LoadFontEx("assets/font/VCR_OSD_MONO_1.001.ttf", 40, NULL, 0);
    SetTextureFilter(myFont_.texture, TEXTURE_FILTER_POINT);
    SetTextLineSpacing(40.0f);

    Vector3 startPos = player_.getPosition();
    scene_.parseCollision("assets/levels/room01_collision.txt");
    scene_.parseLightening("assets/shaders/lighting.txt");
    scene_.loadEnvironment();
    player_.setPosition(startPos);
    player_.syncCamera();
    audioManager_.loadFromManifest("assets/music/audio_manifest.txt");
    audioManager_.parseEmitters("assets/levels/emitters.txt");
    dialogue_.loadDialogue();

    audioManager_.setCurrentZone("OUTDOOR");
    audioManager_.playEmitter("mus_store_sign");
    audioManager_.playMusic("mus_background");
    audioManager_.playMusic("mus_cricket");
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
    Vector3 oldPos = player_.getPosition();

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

    Vector3 newPos = player_.getPosition();

    Vector3 diff = {newPos.x - oldPos.x,
                    newPos.y - oldPos.y,
                    newPos.z - oldPos.z};

    float distanceMoved = std::sqrt(diff.x * diff.x +
                                    diff.y * diff.y +
                                    diff.z * diff.z);

    distanceAccumulator_ += distanceMoved;
    if (distanceAccumulator_ >= stepLength_)
    {
        int num = GetRandomValue(1, 15);
        std::string stepSound = "sfx_step_" + std::to_string(num);
        audioManager_.playSound(stepSound);
        distanceAccumulator_ -= stepLength_;
    }
}

void Game::onZoneChanged()
{
    if (insideStore_)
    {
        audioManager_.playEmitter("mus_refrigerator");
        audioManager_.playEmitter("mus_radio");
        audioManager_.playEmitter("mus_store_lamp");

        audioManager_.muteMusic("mus_store_sign");
        audioManager_.muteMusic("mus_background");
        audioManager_.muteMusic("mus_cricket");
    }
    else
    {
        audioManager_.playEmitter("mus_store_sign");
        audioManager_.playEmitter("mus_test");
        audioManager_.playMusic("mus_background");
        audioManager_.playMusic("mus_cricket");

        audioManager_.muteMusic("mus_refrigerator");
        audioManager_.muteMusic("mus_radio");
        audioManager_.muteMusic("mus_store_lamp");
    }
}

void Game::updateTriggers()
{
    bool wasInside = insideStore_;
    bool isInsideStoreNow = false;

    for (auto &trigger : scene_.getTriggers())
    {
        float cornerX = trigger.position.x - trigger.size.x / 2.0f;
        float cornerZ = trigger.position.z - trigger.size.z / 2.0f;

        Rectangle triggerBox = {cornerX, cornerZ, trigger.size.x, trigger.size.z};
        bool isInsideNow = CheckCollisionCircleRec(
            {player_.getPosition().x, player_.getPosition().z}, player_.getRadius(), triggerBox);

        if (trigger.type == TriggerType::StoreArea && isInsideNow)
        {
            isInsideStoreNow = true;
            if (!trigger.wasInsideLastFrame)
                audioManager_.playSound("sfx_bell");
        }

        if (trigger.type == TriggerType::Teleport && isInsideNow && !trigger.wasInsideLastFrame)
        {
            player_.setPosition(Vector3{12.613f, 0.0f, -27.519});
            player_.setYaw(-PI / 2.0f);
            scene_.resetInteractables();
            loopIteration_++;
        }

        if (!isInsideNow)
        {
            trigger.wasInsideLastFrame = false;
            trigger.active = true;
        }
        else
            trigger.wasInsideLastFrame = true;
    }

    insideStore_ = isInsideStoreNow;

    audioManager_.setCurrentZone(insideStore_ ? "INDOOR" : "OUTDOOR");

    if (wasInside != insideStore_)
        onZoneChanged();
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

        if (dialogue_.getTargetText() != currentPrompt_)
        {
            dialogue_.startConversation(currentPrompt_);
        }

        if (IsKeyPressed(KEY_E) &&
            (scene_.getInteractables()[interactableIndex].type == InteractiveType::Worker))
        {
            scene_.getInteractables()[interactableIndex].active = false;
            state_ = GameState::DIALOGUE;
            // objectiveText_ = "Check for strange noises near fuel pumps";
            dialogue_.startConversation("start");
        }
    }
    else
    {
        if (state_ != GameState::DIALOGUE &&
            dialogue_.getTargetText() != currentPrompt_)
        {
            dialogue_.clear();
        }
    }
}

void Game::update(float dt)
{
    currentPrompt_ = "";

    dialogue_.update(dt, audioManager_);

    if (IsKeyPressed(KEY_K))
        noclipEnabled_ = !noclipEnabled_;

    if (IsKeyPressed(KEY_F3))
        Game::toggleDebugMode();

    if (IsKeyPressed(KEY_P))
    {
        TraceLog(LOG_INFO, "position x: %.3f", player_.getPosition().x);
        TraceLog(LOG_INFO, "position y: %.3f", player_.getPosition().y);
        TraceLog(LOG_INFO, "position z: %.3f", player_.getPosition().z);
    }

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

void Game::renderWorld() const
{
    scene_.renderWorld(player_.getCamera(), isDebugMode_);
}

void Game::renderHud(int screenWidth, int screenHeight)
{
    dialogue_.setStyle(myFont_, screenWidth, screenHeight, 40.0f, WHITE, BLACK, 4.0f);
    DrawCircle(screenWidth / 2, screenHeight / 2, 3.0f, Fade(RAYWHITE, 0.5f));
    DrawRectangle(0, 0, screenWidth, screenHeight, Color{0, 0, 0, 95});

    dialogue_.render();

    if (!objectiveText_.empty())
    {
        DrawTextEx(myFont_, objectiveText_.c_str(), {30.0f, 30.0f}, 20.0f, 1.0f, GOLD);
    }
}
