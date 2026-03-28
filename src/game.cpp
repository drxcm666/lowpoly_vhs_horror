#include "game.hpp"

#include <cmath>
#include <cstddef>

Game::Game() : state_{GameState::START}
{
    Vector3 startPos = player_.getPosition();
    scene_.loadLevelFromTextFile("assets/levels/room01.txt", startPos);
    player_.setPosition(startPos);
    player_.syncCamera();
}

float Game::getInteractionScore(const Vector3 &targetPos, float maxDistance) const
{
    float distance = std::sqrt(
        (player_.getPosition().x - targetPos.x) * (player_.getPosition().x - targetPos.x) +
        (player_.getPosition().z - targetPos.z) * (player_.getPosition().z - targetPos.z));

    if (distance < maxDistance && distance > 0.001f)
    {
        float targetX = (targetPos.x - player_.getPosition().x) / distance;
        float targetZ = (targetPos.z - player_.getPosition().z) / distance;

        float forwardX = player_.getForwardVector().x;
        float forwardZ = player_.getForwardVector().y;

        return (targetX * forwardX + targetZ * forwardZ);
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
    if (!scene_.checkCollisionAllWalls(testPosZ, radius))
        player_.applyMoveZ(stepZ);

    Vector3 testPosX = {currentPos.x + stepX, currentPos.y, currentPos.z};
    if (!scene_.checkCollisionAllWalls(testPosX, radius))
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
                scene_.isPowerOn_ = false;
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

    auto &doors = scene_.getDoors();
    for (std::size_t i = 0; i < doors.size(); i++)
    {
        if (!doors[i].isOpen)
        {
            float score = getInteractionScore(doors[i].position, 3.5f);

            if (score > 0.8f && score > bestScore)
            {
                bestScore = score;
                outDoorIndex = i;
                outInteractableIndex = -1;
            }
        }
    }
}

void Game::handleInteraction(int interactableIndex, int doorIndex)
{
    if (interactableIndex != -1)
    {
        currentPromt_ = scene_.getInteractables()[interactableIndex].promtText;

        if (IsKeyPressed(KEY_E))
        {
            scene_.getInteractables()[interactableIndex].active = false;
            state_ = GameState::FOUND_SWITCH;
        }
    }

    if (doorIndex != -1)
    {
        if (!scene_.getDoors()[doorIndex].isOpen)
        {
            currentPromt_ = scene_.getDoors()[doorIndex].promtText;

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
    currentPromt_ = "";

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
    scene_.renderWorld(player_.getCamera());

    int screenWidth = GetScreenWidth();
    int screenHeight = GetScreenHeight();

    DrawCircle(screenWidth / 2, screenHeight / 2, 3.0f, Fade(RAYWHITE, 0.5f));

    if (!currentPromt_.empty())
    {
        int textWidth = MeasureText(currentPromt_.c_str(), 20);
        DrawText(currentPromt_.c_str(), (screenWidth - textWidth) / 2, screenHeight / 2 - 40, 20, RAYWHITE);
    }
}
