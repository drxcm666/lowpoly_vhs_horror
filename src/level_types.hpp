#pragma once

#include <raylib.h>

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
    Noise,
};

struct TriggerZone
{
    TriggerType type;
    Vector3 position;
    Vector3 size;
    bool active{true};
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