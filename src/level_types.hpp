#pragma once 

#include <raylib.h>

#include <string>


struct Wall
{
    Vector3 position;
    Vector3 size;
};

struct Door
{
    Vector3 position;
    Vector3 size;
    bool isOpen{false};
    std::string promtText;
};

struct TriggerZone
{
    Vector3 position;
    Vector3 size;
    bool active{true};
};

struct Interactable
{
    Vector3 position;
    bool active{true};
    std::string promtText;
};