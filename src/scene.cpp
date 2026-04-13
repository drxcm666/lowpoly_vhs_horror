#include "scene.hpp"
#include "audio.hpp"

#include "rlgl.h"
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <sstream>

Scene::~Scene()
{
    for (const auto &i : models_)
        UnloadModel(i.model);

    UnloadShader(lightShader_);
}

void Scene::loadEnvironment()
{
    for (const auto &i : models_)
        UnloadModel(i.model);

    models_.clear();
    lightSources_.clear();

    lightShader_ = LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
    for (const auto &obj : lightSourcesTxt_)
    {
        Light light;
        if (obj.color == "GOLD")
        {
            light = CreateLight(
                obj.type,
                {obj.position.x, obj.position.y, obj.position.z},
                {0.0f, 0.0f, 0.0f},
                YELLOW,
                lightShader_);
        }
        if (obj.color == "WHITE")
        {
            light = CreateLight(
                obj.type,
                {obj.position.x, obj.position.y, obj.position.z},
                {0.0f, 0.0f, 0.0f},
                WHITE,
                lightShader_);
        }
        lightSources_.push_back(light);
    }
    int ambientLoc = GetShaderLocation(lightShader_, "ambient");
    // float ambientColor[4] = {0.24f, 0.26f, 0.30f, 1.0f};
    float ambientColor[4] = {0.01f, 0.01f, 0.01f, 0.0f};
    SetShaderValue(lightShader_, ambientLoc, ambientColor, SHADER_UNIFORM_VEC4);

    Color nightTint = {95, 92, 105, 255};

    ModelInstance sky{
        LoadModel("assets/sky/sky.glb"),
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {10.0f, 10.0f, 10.0f},
        nightTint,
        false,
        false};
    models_.push_back(sky);

    ModelInstance gasModel{
        LoadModel("assets/gas_station/try.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        false};
    for (int i = 0; i < gasModel.model.materialCount; i++)
    {
        gasModel.model.materials[i].shader = lightShader_;
    }
    models_.push_back(gasModel);

    for (int i = 0; i < 5; i++)
    {
        std::string path = "assets/gas_station/bushes" + std::to_string(i) + ".glb";
        models_.push_back(ModelInstance{
            LoadModel(path.c_str()),
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            nightTint,
            true});
    }

    ModelInstance man{
        LoadModel("assets/man/man.glb"),
        {8.762f, 0.1f, -17.401f},
        {0.0f, 1.0f, 0.0f},
        220.0f,
        {0.6f, 0.6f, 0.6f},
        nightTint,
        false};
    models_.push_back(man);

    // for (const auto &model : models_)
    // {
    //     for (int i = 0; i < model.model.materialCount; i++)
    //     {
    //         model.model.materials[i].shader = lightShader_;
    //     }
    // }

    ModelInstance gasGlassModel{
        LoadModel("assets/gas_station/glass.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        false};
    models_.push_back(gasGlassModel);
}

bool Scene::checkCollision(const Vector3 &playerPos, float playerRadius) const
{
    for (const auto &block : collisionBlocks_)
    {
        float minX = block.position.x - block.size.x / 2.0f;
        float maxX = block.position.x + block.size.x / 2.0f;
        float minY = block.position.y - block.size.y / 2.0f;
        float maxY = block.position.y + block.size.y / 2.0f;
        float minZ = block.position.z - block.size.z / 2.0f;
        float maxZ = block.position.z + block.size.z / 2.0f;

        float closestX = (playerPos.x < minX) ? minX : (playerPos.x > maxX) ? maxX
                                                                            : playerPos.x;
        float closestY = (playerPos.y < minY) ? minY : (playerPos.y > maxY) ? maxY
                                                                            : playerPos.y;
        float closestZ = (playerPos.z < minZ) ? minZ : (playerPos.z > maxZ) ? maxZ
                                                                            : playerPos.z;

        float dx = playerPos.x - closestX;
        float dy = playerPos.y - closestY;
        float dz = playerPos.z - closestZ;
        float distSquared = dx * dx + dy * dy + dz * dz;

        if (distSquared < playerRadius * playerRadius)
            return true;
    }

    return false;
}

void Scene::parseLightening(const std::string &path)
{
    char *lighteningText = LoadFileText(path.c_str());
    if (lighteningText == nullptr)
    {
        TraceLog(LOG_ERROR, "Lightening file not found");
        return;
    }

    std::stringstream rowStream(lighteningText);
    std::string line;

    lightSourcesTxt_.clear();

    while (std::getline(rowStream, line, '\n'))
    {
        if (line.empty() || line.substr(0, 2) == "//")
            continue;

        LightingFixtures object;

        std::stringstream wordStream(line);
        std::string word;
        int wordIndex{0};

        while (wordStream >> word)
        {
            switch (wordIndex)
            {
            case 0:
                object.name = word;
                break;

            case 1:
                object.type = std::stoi(word);
                break;

            case 2:
                object.position.x = std::stof(word);
                break;

            case 3:
                object.position.y = std::stof(word);
                break;

            case 4:
                object.position.z = std::stof(word);
                break;

            case 5:
                object.color = word;
                break;

            default:
                break;
            }

            wordIndex++;
        }

        lightSourcesTxt_.push_back(object);
    }

    UnloadFileText(lighteningText);
}

void Scene::parseCollision(const std::string &path)
{
    char *collisionText = LoadFileText(path.c_str());
    if (collisionText == nullptr)
    {
        TraceLog(LOG_ERROR, "Collision file not found");
        return;
    }

    collisionBlocks_.clear();
    triggers_.clear();
    interactables_.clear();

    std::stringstream rowStream(collisionText);
    std::string line;
    while (std::getline(rowStream, line, '\n'))
    {
        if (line.empty() || line.substr(0, 2) == "//")
            continue;

        CollisionBlock block;

        std::stringstream wordStream(line);
        std::string word;
        int wordIndex{0};
        while (wordStream >> word)
        {
            if (wordIndex == 0)
                block.type = word;

            else if (wordIndex == 1)
                block.position.x = std::stof(word);

            else if (wordIndex == 2)
                block.position.y = std::stof(word);

            else if (wordIndex == 3)
                block.position.z = std::stof(word);

            else if (wordIndex == 4)
                block.size.x = std::stof(word);

            else if (wordIndex == 5)
                block.size.y = std::stof(word);

            else if (wordIndex == 6)
                block.size.z = std::stof(word);

            wordIndex++;
        }

        if (wordIndex < 7)
            continue;

        if (block.type == "WORKER")
        {
            Interactable obj = {InteractiveType::Worker, block.position, true, "To talk"};
            interactables_.push_back(obj);
            collisionBlocks_.push_back(block);
        }
        if (block.type == "STORE_AREA")
        {
            TriggerZone obj = {TriggerType::StoreArea, block.position, block.size, true};
            triggers_.push_back(obj);
        }
        // if (block.type == "FRONT_DOORS")
        // {
        //     TriggerZone obj = {TriggerType::FrontDoors, block.position, block.size, true};
        //     triggers_.push_back(obj);
        // }
        // if (block.type == "BACK_DOORS")
        // {
        //     TriggerZone obj = {TriggerType::BackDoors, block.position, block.size, true};
        //     triggers_.push_back(obj);
        // }
        if (block.type == "WALL" || block.type == "ELSE" ||
            block.type == "BORDERS")
        {
            collisionBlocks_.push_back(block);
        }
    }

    UnloadFileText(collisionText);
}

void Scene::renderWorld(const Camera3D &camera, bool showDebug) const
{
    for (const auto &light : lightSources_)
    {
        UpdateLightValues(lightShader_, light);
    }
    float cameraPos[3] = {camera.position.x, camera.position.y, camera.position.z};
    SetShaderValue(lightShader_, lightShader_.locs[SHADER_LOC_VECTOR_VIEW], cameraPos, SHADER_UNIFORM_VEC3);

    BeginMode3D(camera);

    for (const auto &model : models_)
    {
        if (model.Culling)
        {
            rlDisableBackfaceCulling();
            rlDisableDepthMask();
            DrawModelEx(model.model,
                        model.position,
                        model.rotationAxis,
                        model.rotationAngle, model.scale, model.tint);
            rlEnableDepthMask();
            rlEnableBackfaceCulling();

            continue;
        }
        if (!model.positionLock)
        {
            DrawModelEx(model.model,
                        camera.position,
                        model.rotationAxis,
                        model.rotationAngle, model.scale, model.tint);

            continue;
        }

        DrawModelEx(model.model,
                    model.position,
                    model.rotationAxis,
                    model.rotationAngle, model.scale, model.tint);
    }

    // for (const auto &door : doors_)
    // {
    //     if (!door.isOpen)
    //         DrawCube(door.position, door.size.x, door.size.y, door.size.z, BROWN);
    // }

    if (showDebug)
    {
        for (const auto &block : collisionBlocks_)
        {
            if (block.type == "WALL")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, GREEN);

            if (block.type == "BORDERS")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, RED);

            // if (block.type == "TRIGGER")
            //     DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, YELLOW);

            // if (block.type == "FRONT_DOORS" || block.type == "BACK_DOORS")
            //     DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, ORANGE);

            if (block.type == "INTERACTIVE")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, PINK);

            if (block.type == "ELSE")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLUE);

            if (block.type == "REFRIGERATOR")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLUE);
        }

        for (const auto &trigger : triggers_)
        {
            Color triggerColor = YELLOW;

            // if (trigger.type == TriggerType::FrontDoors)
            //     triggerColor = ORANGE;
            // else if (trigger.type == TriggerType::BackDoors)
            //     triggerColor = MAGENTA;

            DrawCubeWires(trigger.position,
                          trigger.size.x,
                          trigger.size.y,
                          trigger.size.z,
                          triggerColor);
        }
    }

    EndMode3D();
}