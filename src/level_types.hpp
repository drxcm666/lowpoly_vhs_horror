#pragma once

#include <raylib.h>

#include <optional>
#include <string>

struct CollisionBlock
{
    Vector3 position;
    Vector3 size;
    std::string type;
};

struct Door
{
    Vector3 position;
    Vector3 size;
    bool isOpen{false};
    std::string promptText;
};

enum class TriggerType
{
    // FrontDoors,
    // BackDoors,
    StoreArea,
    Noise,
};

struct TriggerZone
{
    TriggerType type;
    Vector3 position;
    Vector3 size;
    bool active{true};
    bool wasInsideLastFrame{false};
};

enum class InteractiveType
{
    Worker,
};

struct Interactable
{
    InteractiveType type;
    Vector3 position;
    bool active{true};
    std::string promptText;
};

struct AudioEmitter
{
    std::string name;
    std::optional<Vector3> position;
    float minDistance;
    float maxDistance;
    float timer{0.0f};
    float minDelay{0.0f};
    float maxDelay{0.0f};
    std::string zone{""};
    bool active{false};
};

struct LightingFixtures
{
    std::string name;
    int type;
    Vector3 position;
    std::string color;
};