#include "scene.hpp"
#include "audio.hpp"

#include "raymath.h"
#include "rlgl.h"
#define RLIGHTS_IMPLEMENTATION
#include "rlights.h"

#include <sstream>

Scene::~Scene()
{
    for (const auto &i : models_)
        UnloadModel(i.model);

    if (lightShader_.id != 0)
    {
        UnloadShader(lightShader_);
    }
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

bool Scene::updateEnvironment(
    int currentLoop, const Vector3 &playerPos,
    float playerRadius, AudioManager &audioManager_,
    bool isLookingAtMonster, float dt)
{
    if (currentLoop != 5)
        return gameOver_;

    ModelInstance *station_m = nullptr;
    ModelInstance *station_case_5_m = nullptr;
    ModelInstance *monster_m = nullptr;
    ModelInstance *monsterSecond_m = nullptr;
    for (auto &model : models_)
    {
        if (model.name == "station")
        {
            station_m = &model;
        }
        if (model.name == "station_case_5")
        {
            station_case_5_m = &model;
        }
        if (model.name == "monster")
        {
            monster_m = &model;
        }
        if (model.name == "monsterSecond")
        {
            monsterSecond_m = &model;
        }
    }

    Interactable *monster_i = nullptr;
    for (auto &itr : interactables_)
    {
        if (itr.type == InteractiveType::Monster)
        {
            monster_i = &itr;
        }
    }

    if (!station_m || !station_case_5_m || !monster_m || !monsterSecond_m || !monster_i)
    {
        TraceLog(LOG_ERROR, "Case 5 environment is incomplete");
        return gameOver_;
    }

    for (auto &trigger : triggers_)
    {
        if (!trigger.active)
            continue;

        float cornerX = trigger.position.x - trigger.size.x / 2.0f;
        float cornerZ = trigger.position.z - trigger.size.z / 2.0f;

        bool isInsideNow = CheckCollisionCircleRec(
            {playerPos.x, playerPos.z}, playerRadius, {cornerX, cornerZ, trigger.size.x, trigger.size.z});
        if (isInsideNow && (trigger.type == TriggerType::InsideDoor))
        {
            trigger.active = false;

            for (auto &block : collisionBlocks_)
            {
                if (block.type == "BLOCK_DOOR" || block.type == "BLOCK_DOOR_2")
                {
                    block.position.y = 0.0f;
                }
            }

            station_m->position.y = -100.0f;
            station_case_5_m->position.y = 0.0f;
            monster_m->position.y = 0.0f;
            monster_m->model.transform = MatrixRotateXYZ({0.0f, 0.0f, 245.0f * DEG2RAD});

            monster_i->position.y = 2.5f;
        }
    }

    if (isLookingAtMonster && !isLookingAtMonsterLastFrame_)
    {
        viewCounts_++;
    }

    if (isLookingAtMonster)
    {
        lookTimer_ += dt;
        isLookingAtMonsterLastFrame_ = true;
    }
    else
    {
        lookTimer_ = 0.0f;
        isLookingAtMonsterLastFrame_ = false;
    }

    if (lookTimer_ >= 2)
    {
        (*monster_m).position.y = -100.0f;
        (*monsterSecond_m).position = {playerPos.x, playerPos.y + 0.1f, playerPos.z - 2.8f};
        monster_i->position = {playerPos.x, playerPos.y + 2.5f, playerPos.z - 2.8f};
        audioManager_.makeSilence();
    }

    if (viewCounts_ == 2)
    {
        if (!jumpscarePlayedOnce_)
        {
            audioManager_.playSound("jumpscare", 1.0f);
            jumpscarePlayedOnce_ = true;
        }

        lookTimer_ += dt;

        if (lookTimer_ >= 3)
        {
            gameOver_ = true;
            jumpscarePlayedOnce_ = false;
            return gameOver_;
        }
    }
    return gameOver_;
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

        std::vector<std::string> tokens;
        while (wordStream >> word)
            tokens.push_back(word);

        try
        {
            if (tokens.size() == 6)
            {
                object.name = tokens[0];
                object.type = std::stoi(tokens[1]);
                object.position.x = std::stoi(tokens[2]);
                object.position.y = std::stoi(tokens[3]);
                object.position.z = std::stoi(tokens[4]);
                object.color = tokens[5];
                object.baseColor = tokens[5];
            }
        }
        catch (const std::exception &)
        {
            continue;
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
        std::vector<std::string> tokens;
        std::string word;
        while (wordStream >> word)
            tokens.push_back(word);

        try
        {
            if (tokens.size() == 7)
            {
                // type px py pz sx sy sz
                block.type = tokens[0];
                block.position.x = std::stof(tokens[1]);
                block.position.y = std::stof(tokens[2]);
                block.position.z = std::stof(tokens[3]);
                block.size.x = std::stof(tokens[4]);
                block.size.y = std::stof(tokens[5]);
                block.size.z = std::stof(tokens[6]);
                block.basePosition.x = block.position.x;
                block.basePosition.y = block.position.y;
                block.basePosition.z = block.position.z;
                block.baseSize.x = block.size.x;
                block.baseSize.y = block.size.y;
                block.baseSize.z = block.size.z;
            }
            else
            {
                continue;
            }
        }
        catch (const std::exception &)
        {
            continue;
        }

        if (block.type == "WORKER")
        {
            Interactable obj = {InteractiveType::Worker, block.position, true, "To talk", "first_meeting", block.position};
            interactables_.push_back(obj);
            collisionBlocks_.push_back(block);
        }
        if (block.type == "MONSTER")
        {
            Interactable obj = {InteractiveType::Monster, block.position, true, "", "", block.position};
            interactables_.push_back(obj);
        }
        if (block.type == "STORE_AREA")
        {
            TriggerZone obj = {TriggerType::StoreArea, block.position, block.size, true};
            triggers_.push_back(obj);
        }
        if (block.type == "DOOR_5_CASE")
        {
            TriggerZone obj = {TriggerType::InsideDoor, block.position, block.size, true};
            triggers_.push_back(obj);
        }
        if (block.type == "TELEPORT_TRIGGER")
        {
            TriggerZone obj = {TriggerType::Teleport, block.position, block.size, false};
            triggers_.push_back(obj);
        }
        if (block.type == "WALL" || block.type == "ELSE" ||
            block.type == "BORDERS" || block.type == "SHELF" ||
            block.type == "COFFEE" || block.type == "ICECREAM" ||
            block.type == "TABLE" || block.type == "BLOCK_DOOR" ||
            block.type == "BLOCK_DOOR_2" || block.type == "BLOCK_BACK_DOOR")
        {
            collisionBlocks_.push_back(block);
        }
    }

    UnloadFileText(collisionText);
}

void Scene::resetInteractables()
{
    for (auto &t : triggers_)
    {
        t.active = true;
    }
    for (auto &i : interactables_)
    {
        i.active = true;
    }
}

void Scene::resetEnvironment(int currentLoop, AudioManager &audioManager_)
{
    Texture2D *floorPtr = nullptr;
    Texture2D *ceilingPtr = nullptr;
    for (auto &model : models_)
    {
        if (model.name == "floor")
            floorPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;

        if (model.name == "ceiling")
            ceilingPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
    }
    for (auto &model : models_)
    {
        model.position = model.basePosition;
        model.tint = model.baseTint;
        model.rotationAngle = model.baseRotationAngle;
        model.model.transform = MatrixIdentity();

        if (model.name == "floor")
            floorPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;

        if (model.name == "ceiling")
            ceilingPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
    }
    if (floorPtr && ceilingPtr && (currentLoop == 4))
    {
        std::swap(*floorPtr, *ceilingPtr);
    }

    for (auto &block : collisionBlocks_)
    {
        block.position = block.basePosition;
        block.size = block.baseSize;
    }
    for (auto &itr : interactables_)
    {
        itr.position = itr.basePosition;
    }
    for (size_t i = 0; i < lightSources_.size(); i++)
    {
        if (i < baseLightColors_.size())
        {
            lightSources_[i].color = baseLightColors_[i];
        }
    }

    int ambientLoc = GetShaderLocation(lightShader_, "ambient");
    float ambientColor[4] = {0.01f, 0.01f, 0.01f, 0.0f};
    SetShaderValue(lightShader_, ambientLoc, ambientColor, SHADER_UNIFORM_VEC4);

    for (auto &music : audioManager_.getMusic())
    {
        SetMusicPitch(music.second, 1.0f);
    }
}

void Scene::applyMutations(int currentLoop, AudioManager &audioManager)
{
    resetEnvironment(currentLoop, audioManager);

    switch (currentLoop)
    {
    case 1:
    {
        for (auto &model : models_)
        {
            if (model.name == "cashier")
            {
                model.position = {8.4f, 0.1f, -20.88};
                model.rotationAngle = 180.0f;
            }
        }
        for (auto &block : collisionBlocks_)
        {
            if (block.type == "WORKER")
            {
                block.position = {8.4f, 1.0f, -20.88};
            }
        }
        for (auto &itr : interactables_)
        {
            if (itr.type == InteractiveType::Worker)
            {
                itr.position = {8.4f, 1.0f, -20.88};
            }
        }
    }
    break;

    case 2:
    {
        audioManager.makeSilence();

        for (auto &itr : interactables_)
        {
            if (itr.type == InteractiveType::Worker)
            {
                itr.dialogueNodeID = "silent_return";
            }
        }
    }
    break;

    case 3:
    {
        Texture2D *floorPtr = nullptr;
        Texture2D *ceilingPtr = nullptr;
        for (auto &model : models_)
        {
            if (model.name == "table")
            {
                model.rotationAxis = {1.0f, 0.0f, 0.0f};
                model.rotationAngle = 180.0f;
                model.position = {8.0f, 2.9f, -16.38f};
            }
            if (model.name == "icecream")
            {
                model.rotationAxis = {1.0f, 0.0f, 0.0f};
                model.rotationAngle = 180.0f;
                model.position = {6.23f, 3.2f, -24.0f};
            }
            if (model.name == "coffee")
            {
                model.rotationAxis = {1.0f, 0.0f, 0.0f};
                model.rotationAngle = 180.0f;
                model.position = {9.7f, 2.3f, -22.9f};
            }
            if (model.name == "shelves")
            {
                model.rotationAxis = {0.0f, 0.0f, 1.0f};
                model.rotationAngle = 180.0f;
                model.position = {3.1f, 3.05f, -19.45f};
            }
            if (model.name == "refrigerator")
            {
                model.rotationAxis = {1.0f, 0.0f, 0.0f};
                model.rotationAngle = 180.0f;
                model.position = {-1.33f, 1.3f, -18.555f};
            }
            if (model.name == "tv")
            {
                model.rotationAxis = {0.0f, 0.0f, 1.0f};
                model.rotationAngle = 180.0f;
                model.position = {8.34f, 1.1f, -18.43f};
            }
            if (model.name == "lamps")
            {
                model.rotationAxis = {0.0f, 0.0f, 1.0f};
                model.rotationAngle = 180.0f;
                model.position = {5.2f, 0.2f, -19.487f};
            }
            if (model.name == "camera_1")
            {
                model.rotationAxis = {0.0f, 0.0f, 1.0f};
                model.rotationAngle = 180.0f;
                model.position = {-0.3f, 1.0f, -24.25f};
            }
            if (model.name == "camera_2")
            {
                model.rotationAxis = {1.0f, 0.0f, 0.0f};
                model.rotationAngle = 180.0f;
                model.position = {10.0f, 1.0f, -18.5f};
            }
            if (model.name == "desk")
            {
                model.rotationAxis = {0.0f, 0.0f, 1.0f};
                model.rotationAngle = 180.0f;
                model.position = {10.2f, 3.2f, -17.1f};
            }
            if (model.name == "cashier")
            {
                Matrix rotY = MatrixRotateY(175.0f * DEG2RAD);
                Matrix rotX = MatrixRotateX(180.0f * DEG2RAD);
                model.model.transform = MatrixMultiply(rotY, rotX);
                model.position = model.basePosition;
                model.position.y = 4.0f;
            }

            if (model.name == "floor")
                floorPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;

            if (model.name == "ceiling")
                ceilingPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
        }

        if (floorPtr && ceilingPtr)
        {
            std::swap(*floorPtr, *ceilingPtr);
        }

        for (auto &itr : interactables_)
        {
            if (itr.type == InteractiveType::Worker)
            {
                itr.dialogueNodeID = "upside_down_meeting";
                itr.position.y = 3.0f;
            }
        }

        for (auto &col : collisionBlocks_)
        {
            if (col.type == "SHELF" || col.type == "ICECREAM" ||
                col.type == "TABLE")
            {
                col.position.y = -10.0f;
            }
            if (col.type == "COFFEE")
            {
                col.position.x += 0.3f;
            }
            if (col.type == "WORKER")
            {
                col.position.y = 3.0f;
            }
        }
    }
    break;

    case 4:
    {
        int ambientLoc = GetShaderLocation(lightShader_, "ambient");
        float ambientColor[4] = {1.0f, 0.01f, 0.01f, 0.0f};
        SetShaderValue(lightShader_, ambientLoc, ambientColor, SHADER_UNIFORM_VEC4);

        for (auto &light : lightSources_)
        {
            light.color = RED;
        }

        // Texture2D *floorPtr = nullptr;
        // Texture2D *ceilingPtr = nullptr;
        for (auto &model : models_)
        {
            Color redTint = {65, 10, 10, 255};
            model.tint = redTint;

            // if (model.name == "floor")
            //     floorPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;

            // if (model.name == "ceiling")
            //     ceilingPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
        }
        // if (floorPtr && ceilingPtr)
        // {
        //     std::swap(*floorPtr, *ceilingPtr);
        // }

        float redPitch = 0.3f;
        for (auto &music : audioManager.getMusic())
        {
            SetMusicPitch(music.second, redPitch);
        }

        for (auto &itr : interactables_)
        {
            if (itr.type == InteractiveType::Worker)
            {
                itr.dialogueNodeID = "blood";
            }
        }
    }
    break;

    case 5:
    {
        for (auto &model : models_)
        {
            if (model.name == "cashier")
            {
                model.position.y = -100.0f;
            }
        }
        for (auto &block : collisionBlocks_)
        {
            if (block.type == "WORKER")
            {
                block.position.y = -100.0f;
            }
        }
        for (auto &itr : interactables_)
        {
            if (itr.type == InteractiveType::Worker)
            {
                itr.position.y = -100.0f;
            }
        }
    }
    break;

    default:
    {
        resetEnvironment(currentLoop, audioManager);
    }
    break;
    }
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

    if (showDebug)
    {
        for (const auto &block : collisionBlocks_)
        {
            if (block.type == "WALL" || block.type == "BLOCK_BACK_DOOR" || block.type == "MONSTER")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, GREEN);

            if (block.type == "BORDERS")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, RED);

            if (block.type == "INTERACTIVE")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, PINK);

            if (block.type == "WORKER")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, RED);

            if (block.type == "ELSE")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLUE);

            if (block.type == "REFRIGERATOR")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLUE);
        }

        for (const auto &trigger : triggers_)
        {
            Color triggerColor = YELLOW;

            DrawCubeWires(trigger.position,
                          trigger.size.x,
                          trigger.size.y,
                          trigger.size.z,
                          triggerColor);
        }
    }

    EndMode3D();
}