#include "scene.hpp"
#include "audio.hpp"

#include "rlgl.h"

#include <sstream>

Scene::~Scene()
{
    // UnloadShader(lightingShader_);

    for (const auto &i : models_)
        UnloadModel(i.model);
}

void Scene::loadEnvironment()
{
    for (const auto &i : models_)
        UnloadModel(i.model);

    models_.clear();

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
    models_.push_back(gasModel);

    ModelInstance gasBushesModel0{
        LoadModel("assets/gas_station/bushes0.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        true};
    models_.push_back(gasBushesModel0);

    ModelInstance gasBushesModel1{
        LoadModel("assets/gas_station/bushes1.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        true};
    models_.push_back(gasBushesModel1);

    ModelInstance gasBushesModel2{
        LoadModel("assets/gas_station/bushes2.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        true};
    models_.push_back(gasBushesModel2);

    ModelInstance gasBushesModel3{
        LoadModel("assets/gas_station/bushes3.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        true};
    models_.push_back(gasBushesModel3);

    ModelInstance gasBushesModel4{
        LoadModel("assets/gas_station/bushes4.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        true};
    models_.push_back(gasBushesModel4);

    ModelInstance gasGlassModel{
        LoadModel("assets/gas_station/glass.glb"),
        {0.0f, 0.0f, 0.0f},
        {0.0f, 0.0f, 1.0f},
        0.0f,
        {1.0f, 1.0f, 1.0f},
        nightTint,
        false};
    models_.push_back(gasGlassModel);

    ModelInstance man{
        LoadModel("assets/man/man.glb"),
        {8.762f, 0.1f, -17.401f},
        {0.0f, 1.0f, 0.0f},
        220.0f,
        {0.6f, 0.6f, 0.6f},
        nightTint,
        false};
    models_.push_back(man);

    // Mesh skyMesh = GenMeshSphere(20.0f, 64, 64);
    // skyModel_ = LoadModelFromMesh(skyMesh);
    // skyTexture_ = LoadTexture("assets/sky3.png");
    // skyModel_.materials[0].maps[MATERIAL_MAP_DIFFUSE].texture = skyTexture_;
    // skyModel_.materials[0].params[MATERIAL_MAP_METALNESS] = 0;
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
        collisionBlocks_.push_back(block);

        if (block.type == "WORKER")
        {
            Interactable obj = {InteractiveType::Worker, block.position, true, "To talk"};
            interactables_.push_back(obj);
        }
        if (block.type == "TRIGGER")
        {
            TriggerZone obj = {TriggerType::Noise, block.position, block.size, true};
            triggers_.push_back(obj);
        }
        if (block.type == "FRONT_DOORS")
        {
            TriggerZone obj = {TriggerType::FrontDoors, block.position, block.size, true};
            triggers_.push_back(obj);
        }
        if (block.type == "BACK_DOORS")
        {
            TriggerZone obj = {TriggerType::BackDoors, block.position, block.size, true};
            triggers_.push_back(obj);
        }
    }

    UnloadFileText(collisionText);
}

void Scene::renderWorld(const Camera3D &camera, bool showDebug) const
{
    // Shader uniforms are temporarily disabled.
    // float lampPosArray[3] = {lampPosition_.x, lampPosition_.y, lampPosition_.z};
    // SetShaderValue(lightingShader_, lightPosLoc_, lampPosArray, SHADER_UNIFORM_VEC3);
    // float cameraPosArray[3] = {camera.position.x, camera.position.y, camera.position.z};
    // SetShaderValue(lightingShader_, viewPosLoc_, cameraPosArray, SHADER_UNIFORM_VEC3);

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

    for (const auto &door : doors_)
    {
        if (!door.isOpen)
            DrawCube(door.position, door.size.x, door.size.y, door.size.z, BROWN);
    }

    if (showDebug)
    {
        for (const auto &block : collisionBlocks_)
        {
            if (block.type == "WALL")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, GREEN);

            if (block.type == "BORDERS")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, RED);

            if (block.type == "TRIGGER")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, YELLOW);

            if (block.type == "FRONT_DOORS" || block.type == "BACK_DOORS")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, ORANGE);

            if (block.type == "INTERACTIVE")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, PINK);

            if (block.type == "ELSE")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLUE);

            if (block.type == "REFRIGERATOR")
                DrawCubeWires(block.position, block.size.x, block.size.y, block.size.z, BLUE);
        }
    }

    EndMode3D();

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 95});
}
