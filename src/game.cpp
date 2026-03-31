#include "game.hpp"
#include "scene.hpp"

#include <cmath>
#include <cstddef>

bool Game::isDebugMode_ = false;

Game::Game() : state_{GameState::START}, objectiveText_{"Go inside the gas station"}
{
    Vector3 startPos = player_.getPosition();
    scene_.loadCollisionFile("assets/levels/room01_collision.txt");
    scene_.loadEnvironment();
    player_.setPosition(startPos);
    player_.syncCamera();
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
        if (trigger.active)
        {
            float cornerX = trigger.position.x - trigger.size.x / 2.0f;
            float cornerZ = trigger.position.z - trigger.size.z / 2.0f;
            Rectangle triggerBox = {cornerX, cornerZ, trigger.size.x, trigger.size.z};
            if (CheckCollisionCircleRec(
                    {player_.getPosition().x, player_.getPosition().z}, player_.getRadius(), triggerBox))
            {
                trigger.active = false;
            }
        }
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

    // auto &doors = scene_.getDoors();
    // for (std::size_t i = 0; i < doors.size(); i++)
    // {
    //     if (!doors[i].isOpen)
    //     {
    //         float score = getInteractionScore(doors[i].position, 3.5f);

    //         if (score > 0.8f && score > bestScore)
    //         {
    //             bestScore = score;
    //             outDoorIndex = i;
    //             outInteractableIndex = -1;
    //         }
    //     }
    // }
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

    if (doorIndex != -1)
    {
        if (!scene_.getDoors()[doorIndex].isOpen)
        {
            currentPrompt_ = scene_.getDoors()[doorIndex].promptText;

            if (IsKeyPressed(KEY_E))
            {
                scene_.getDoors()[doorIndex].isOpen = true;
                state_ = GameState::ESCAPED;
            }
        }
    }
}

void Game::update(float dt)
{
    currentPrompt_ = "";

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
