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

    UnloadShader(lightShader_);
}

void Scene::loadEnvironment()
{
    for (const auto &i : models_)
        UnloadModel(i.model);

    models_.clear();
    lightSources_.clear();
    baseLightColors_.clear();

    lightShader_ = LoadShader("assets/shaders/lighting.vs", "assets/shaders/lighting.fs");
    
    for (const auto &obj : lightSourcesTxt_)
    {
        Light light;
        Color baseColor = WHITE;
        if (obj.color == "GOLD")
        {
            light = CreateLight(
                obj.type,
                {obj.position.x, obj.position.y, obj.position.z},
                {0.0f, 0.0f, 0.0f},
                ORANGE,
                lightShader_);
            baseColor = ORANGE;
        }
        if (obj.color == "WHITE")
        {
            light = CreateLight(
                obj.type,
                {obj.position.x, obj.position.y, obj.position.z},
                {0.0f, 0.0f, 0.0f},
                WHITE,
                lightShader_);
            baseColor = WHITE;
        }
        lightSources_.push_back(light);
        baseLightColors_.push_back(baseColor);
    }
    int ambientLoc = GetShaderLocation(lightShader_, "ambient");
    float ambientColor[4] = {0.01f, 0.01f, 0.01f, 0.0f};
    SetShaderValue(lightShader_, ambientLoc, ambientColor, SHADER_UNIFORM_VEC4);

    Color nightTint = {95, 92, 105, 255};

    ModelInstance sky{
        "sky",
        LoadModel("assets/sky/sky.glb"),
        {0.0f, 1.0f, 0.0f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {10.0f, 10.0f, 10.0f},
        nightTint,
        false,
        false,
        {0.0f, 1.0f, 0.0f},
        0.0f,
        nightTint};
    models_.push_back(sky);

    auto addStationPart = [this, &nightTint](const char *name, const char *path)
    {
        ModelInstance part{
            name,
            LoadModel(path),
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            nightTint,
            false,
            true,
            {0.0f, 0.0f, 0.0f},
            0.0f,
            nightTint};

        for (int i = 0; i < part.model.materialCount; i++)
        {
            part.model.materials[i].shader = lightShader_;
        }

        models_.push_back(part);
    };

    addStationPart("station", "assets/gas_station/station.glb");
    addStationPart("ceiling", "assets/gas_station/ceiling.glb");
    addStationPart("floor", "assets/gas_station/floor.glb");

    for (int i = 0; i < 5; i++)
    {
        std::string path = "assets/gas_station/bushes" + std::to_string(i) + ".glb";
        ModelInstance bush{
            "bushes_" + std::to_string(i),
            LoadModel(path.c_str()),
            {0.0f, 0.0f, 0.0f},
            {0.0f, 0.0f, 1.0f},
            0.0f,
            {1.0f, 1.0f, 1.0f},
            nightTint,
            true,
            true,
            {0.0f, 0.0f, 0.0f},
            0.0f,
            nightTint};

        for (int i = 0; i < bush.model.materialCount; i++)
        {
            bush.model.materials[i].shader = lightShader_;
        }
        models_.push_back(bush);
    }

    ModelInstance monster{
        "monster",
        LoadModel("assets/monster/monster.glb"),
        {13.46f, -100.0f, 12.3f},
        {1.0f, 0.0f, 0.0f},
        90.0f,
        {0.7f, 0.7f, 0.7f},
        nightTint,
        false,
        true,
        {13.46f, -100.0f, 12.3f},
        90.0f,
        nightTint};
    for (int i = 0; i < monster.model.materialCount; i++)
    {
        monster.model.materials[i].shader = lightShader_;
    }
    models_.push_back(monster);

    ModelInstance station_case_5{
        "station_case_5",
        LoadModel("assets/gas_station/station_case_5.glb"),
        {0.0f, -100.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        false,
        true,
        {0.0f, -100.0f, 0.0f},
        0.0f,
        nightTint};
    for (int i = 0; i < station_case_5.model.materialCount; i++)
    {
        station_case_5.model.materials[i].shader = lightShader_;
    }
    models_.push_back(station_case_5);

    ModelInstance desk{
        "desk",
        LoadModel("assets/gas_station/desk.glb"),
        {10.2f, 2.2f, -17.1f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {10.2f, 2.2f, -17.1f},
        0.0f,
        nightTint};
    for (int i = 0; i < desk.model.materialCount; i++)
    {
        desk.model.materials[i].shader = lightShader_;
    }
    models_.push_back(desk);

    ModelInstance camera_2{
        "camera_2",
        LoadModel("assets/gas_station/camera_2.glb"),
        {10.0f, 3.6f, -18.5f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {10.0f, 3.6f, -18.5f},
        0.0f,
        nightTint};
    for (int i = 0; i < camera_2.model.materialCount; i++)
    {
        camera_2.model.materials[i].shader = lightShader_;
    }
    models_.push_back(camera_2);

    ModelInstance camera_1{
        "camera_1",
        LoadModel("assets/gas_station/camera_1.glb"),
        {-0.3f, 3.6f, -24.25f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {-0.3f, 3.6f, -24.25f},
        0.0f,
        nightTint};
    for (int i = 0; i < camera_1.model.materialCount; i++)
    {
        camera_1.model.materials[i].shader = lightShader_;
    }
    models_.push_back(camera_1);

    ModelInstance lamps{
        "lamps",
        LoadModel("assets/gas_station/lamps.glb"),
        {5.2f, 4.0f, -19.487f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {5.2f, 4.0f, -19.487f},
        0.0f,
        nightTint};
    for (int i = 0; i < lamps.model.materialCount; i++)
    {
        lamps.model.materials[i].shader = lightShader_;
    }
    models_.push_back(lamps);

    ModelInstance tv{
        "tv",
        LoadModel("assets/gas_station/tv.glb"),
        {8.34f, 3.1f, -18.43f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {8.34f, 3.1f, -18.43f},
        0.0f,
        nightTint};
    for (int i = 0; i < tv.model.materialCount; i++)
    {
        tv.model.materials[i].shader = lightShader_;
    }
    models_.push_back(tv);

    ModelInstance refrigerator{
        "refrigerator",
        LoadModel("assets/gas_station/refrigerator.glb"),
        {-1.33f, 1.6f, -18.83f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {-1.33f, 1.6f, -18.83f},
        0.0f,
        nightTint};
    for (int i = 0; i < refrigerator.model.materialCount; i++)
    {
        refrigerator.model.materials[i].shader = lightShader_;
    }
    models_.push_back(refrigerator);

    ModelInstance shelves{
        "shelves",
        LoadModel("assets/gas_station/shelves.glb"),
        {2.5f, 1.2f, -19.45f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {2.5f, 1.2f, -19.45f},
        0.0f,
        nightTint};
    for (int i = 0; i < shelves.model.materialCount; i++)
    {
        shelves.model.materials[i].shader = lightShader_;
    }
    models_.push_back(shelves);

    ModelInstance coffee{
        "coffee",
        LoadModel("assets/gas_station/coffee.glb"),
        {9.7f, 1.8f, -23.12f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {9.7f, 1.8f, -23.12f},
        0.0f,
        nightTint};
    for (int i = 0; i < coffee.model.materialCount; i++)
    {
        coffee.model.materials[i].shader = lightShader_;
    }
    models_.push_back(coffee);

    ModelInstance icecream{
        "icecream",
        LoadModel("assets/gas_station/icecream.glb"),
        {6.23f, 1.0f, -24.0f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.00f},
        nightTint,
        false,
        true,
        {6.23f, 1.0f, -24.0f},
        0.0f,
        nightTint};
    for (int i = 0; i < icecream.model.materialCount; i++)
    {
        icecream.model.materials[i].shader = lightShader_;
    }
    models_.push_back(icecream);

    ModelInstance table{
        "table",
        LoadModel("assets/gas_station/table1.glb"),
        {8.0f, 1.3f, -16.8f},
        {0.0f, 1.0f, 0.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        false,
        true,
        {8.0f, 1.3f, -16.8f},
        0.0f,
        nightTint};
    for (int i = 0; i < table.model.materialCount; i++)
    {
        table.model.materials[i].shader = lightShader_;
    }
    models_.push_back(table);

    ModelInstance cashier{
        "cashier",
        LoadModel("assets/cashier/test1.glb"),
        {8.762f, 0.1f, -17.401f},
        {0.0f, 1.0f, 0.0f},
        265.0f,
        {0.42f, 0.42f, 0.42f},
        nightTint,
        false,
        true,
        {8.762f, 0.1f, -17.401f},
        265.0f,
        nightTint};
    for (int i = 0; i < cashier.model.materialCount; i++)
    {
        cashier.model.materials[i].shader = lightShader_;
    }
    models_.push_back(cashier);

    ModelInstance car{
        "car",
        LoadModel("assets/car/car.glb"),
        {10.0f, 0.3f, 5.5f},
        {0.0f, 1.0f, 0.0f},
        90.0f,
        {1.5f, 1.5f, 1.5f},
        nightTint,
        false,
        true,
        {10.0f, 0.3f, 5.5f},
        90.0f,
        nightTint};
    for (int i = 0; i < car.model.materialCount; i++)
    {
        car.model.materials[i].shader = lightShader_;
    }
    models_.push_back(car);

    ModelInstance gasGlassModel{
        "glass",
        LoadModel("assets/gas_station/glass.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        false,
        true,
        {0.0f, 0.0f, 0.0f},
        0.0f,
        nightTint};
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

void Scene::updateEnvironment(int currentLoop, const Vector3 &playerPos, float playerRadius)
{
    if (currentLoop == 5)
    {
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
                for (auto &model : models_)
                {
                    if (model.name == "station")
                    {
                        model.position.y = -100.0f;
                    }
                    if (model.name == "station_case_5")
                    {
                        model.position.y = 0.0f;
                    }
                    if (model.name == "monster")
                    {
                        model.position.y = 0.0f;
                        model.model.transform = MatrixRotateXYZ({0.0f, 0.0f, 245.0f * DEG2RAD});
                    }
                }
            }
        }
    }
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
            {
                object.color = word;
                object.baseColor = word;
            }
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

void Scene::resetEnvironment(AudioManager &audioManager_)
{
    for (auto &mdl : models_)
    {
        mdl.position = mdl.basePosition;
        mdl.tint = mdl.baseTint;
        mdl.rotationAngle = mdl.baseRotationAngle;
        mdl.model.transform = MatrixIdentity();
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

void Scene::applyMutations(int currentLoop, AudioManager &audioManager_)
{
    resetEnvironment(audioManager_);

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
        auto &emtMap = audioManager_.getEmitters();
        for (auto &emt : emtMap)
        {
            emt.second.active = false;
        }
        auto &musMap = audioManager_.getMusic();
        for (auto &music : musMap)
        {
            if (IsMusicStreamPlaying(music.second))
            {
                audioManager_.stopMusic(music.first);
            }
        }
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
            {
                floorPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
            }
            if (model.name == "ceiling")
            {
                ceilingPtr = &model.model.materials[1].maps[MATERIAL_MAP_ALBEDO].texture;
            }
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
        for (auto &model : models_)
        {
            Color redTint = {65, 10, 10, 255};
            model.tint = redTint;
        }
        float redPitch = 0.3f;
        for (auto &music : audioManager_.getMusic())
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
        resetEnvironment(audioManager_);
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
            if (block.type == "WALL" || block.type == "BLOCK_BACK_DOOR")
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