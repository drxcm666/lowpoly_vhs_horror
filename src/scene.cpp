#include "scene.hpp"

#include "rlgl.h"

void Scene::loadLevelFromTextFile(const std::string &path, Vector3 &playerPosition)
{
    walls_.clear();
    doors_.clear();
    triggers_.clear();
    interactables_.clear();
    isPowerOn_ = true;

    gasModel_ = LoadModel("assets/gas_station/gas.glb");
    gasGlassModel_ = LoadModel("assets/gas_station/glass.glb");
    gasBushesModel0_ = LoadModel("assets/gas_station/bushes0.glb");
    gasBushesModel1_ = LoadModel("assets/gas_station/bushes1.glb");
    gasBushesModel2_ = LoadModel("assets/gas_station/bushes2.glb");
    gasBushesModel3_ = LoadModel("assets/gas_station/bushes3.glb");
    gasBushesModel4_ = LoadModel("assets/gas_station/bushes4.glb");

    // Shader pipeline is temporarily disabled.
    // lightingShader_ = LoadShader("assets/gas_station/shaders/lighting.vs",
    //                              "assets/gas_station/shaders/lighting.fs");
    // lightPosLoc_ = GetShaderLocation(lightingShader_, "lightPos");
    // lightColorLoc_ = GetShaderLocation(lightingShader_, "lightColor");
    // viewPosLoc_ = GetShaderLocation(lightingShader_, "viewPos");
    //
    // for (int i = 0; i < gasModel_.materialCount; ++i)
    // {
    //     gasModel_.materials[i].shader = lightingShader_;
    // }
    //
    // lampPosition_ = {25.0f, 5.0f, 25.0f};
    // float lightColor[3] = {1.0f, 0.82f, 0.58f};
    // SetShaderValue(lightingShader_, lightColorLoc_, lightColor, SHADER_UNIFORM_VEC3);

    levelWidthTiles_ = 0;
    levelHeightTiles_ = 0;

    bool playerFound{false};

    char *mapText = LoadFileText(path.c_str());
    if (mapText == nullptr)
    {
        TraceLog(LOG_ERROR, "Map file not found");
        return;
    }

    int i{0}, col{0}, row{0};

    while (mapText[i] != '\0')
    {
        char cell = mapText[i];

        switch (cell)
        {
        case '\n':
            row++;
            col = 0;
            break;

        case '.':
            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;

        case '#':
        {
            float worldZ = row * tileSize_;
            float worldX = col * tileSize_;

            walls_.push_back({{worldX, 2.0f, worldZ}, {tileSize_, tileSize_, tileSize_}});

            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;
        }

        case 'P':
        {
            playerFound = true;

            float worldZ = row * tileSize_;
            float worldX = col * tileSize_;

            playerPosition.z = worldZ;
            playerPosition.x = worldX;

            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;
        }

        case 'T':
        {
            float worldZ = row * tileSize_;
            float worldX = col * tileSize_;

            triggers_.push_back({{worldX, 1.0f, worldZ}, {1.0f, 1.0f, 1.0f}});

            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;
        }

        case 'I':
        {
            float worldZ = row * tileSize_;
            float worldX = col * tileSize_;

            interactables_.push_back({{worldX, 1.0f, worldZ}, true, "Press E to interact"});

            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;
        }

        case 'D':
        {
            float worldZ = row * tileSize_;
            float worldX = col * tileSize_;

            doors_.push_back({{worldX, 2.0f, worldZ},
                              {tileSize_, tileSize_, tileSize_},
                              false,
                              "Press E to open door"});

            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;
        }

        case '\r':
            break;

        default:
            col++;
            if (col > levelWidthTiles_)
                levelWidthTiles_ = col;

            if ((row + 1) > levelHeightTiles_)
                levelHeightTiles_ = row + 1;

            break;
        }
        i++;
    }

    UnloadFileText(mapText);
    if (!playerFound)
    {
        TraceLog(LOG_ERROR, "Player start (P) not found. Using default");
        playerPosition = {0.0f, 0.0f, 0.0f};
    }
}

Scene::~Scene()
{
    UnloadModel(gasModel_);
    UnloadModel(gasGlassModel_);
    UnloadModel(gasBushesModel0_);
    UnloadModel(gasBushesModel1_);
    UnloadModel(gasBushesModel2_);
    UnloadModel(gasBushesModel3_);
    UnloadModel(gasBushesModel4_);
    // UnloadShader(lightingShader_);
}

bool Scene::checkCollisionSingleWall(const Vector3 &playerPos, const Wall &wall, float playerRadius) const
{
    float cornerX = wall.position.x - wall.size.x / 2.0f;
    float cornerZ = wall.position.z - wall.size.z / 2.0f;
    Rectangle wallBox = {cornerX, cornerZ, wall.size.x, wall.size.z};

    return CheckCollisionCircleRec({playerPos.x, playerPos.z}, playerRadius, wallBox);
}

bool Scene::checkCollisionAllWalls(const Vector3 &playerPos, float playerRadius) const
{
    for (const auto &wall : walls_)
    {
        if (checkCollisionSingleWall(playerPos, wall, playerRadius))
            return true;
    }

    for (const auto &door : doors_)
    {
        if (!door.isOpen)
        {
            float cornerX = door.position.x - door.size.x / 2.0f;
            float cornerZ = door.position.z - door.size.z / 2.0f;
            Rectangle doorBox = {cornerX, cornerZ, door.size.x, door.size.z};

            if (CheckCollisionCircleRec({playerPos.x, playerPos.z}, playerRadius, doorBox))
                return true;
        }
    }

    return false;
}

void Scene::renderWorld(Camera3D camera) const
{
    float floorSizeZ{levelHeightTiles_ * tileSize_};
    float floorSizeX{levelWidthTiles_ * tileSize_};

    float floorCenterZ{((levelHeightTiles_ - 1) * tileSize_) / 2.0f};
    float floorCenterX{((levelWidthTiles_ - 1) * tileSize_) / 2.0f};

    Color wallColor = isPowerOn_ ? Fade(GRAY, 0.3f) : DARKGRAY;
    Color floorColor = isPowerOn_ ? Fade(LIGHTGRAY, 0.3f) : GRAY;

    // Shader uniforms are temporarily disabled.
    // float lampPosArray[3] = {lampPosition_.x, lampPosition_.y, lampPosition_.z};
    // SetShaderValue(lightingShader_, lightPosLoc_, lampPosArray, SHADER_UNIFORM_VEC3);
    // float cameraPosArray[3] = {camera.position.x, camera.position.y, camera.position.z};
    // SetShaderValue(lightingShader_, viewPosLoc_, cameraPosArray, SHADER_UNIFORM_VEC3);

    BeginMode3D(camera);

    Color nightTint = {210, 205, 190, 255};
    DrawModelEx(gasModel_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);

    rlDisableBackfaceCulling();
    rlDisableDepthMask();
    DrawModelEx(gasBushesModel0_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);
    DrawModelEx(gasBushesModel1_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);
    DrawModelEx(gasBushesModel2_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);
    DrawModelEx(gasBushesModel3_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);
    DrawModelEx(gasBushesModel4_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);
    rlEnableDepthMask();
    rlEnableBackfaceCulling();

    DrawModelEx(gasGlassModel_, {20.0f, 0.0f, 20.0f}, {0.0f, 0.0f, 1.0f}, 0.0f, {1.0f, 1.0f, 1.0f}, nightTint);

    for (const auto &wall : walls_)
    {
        DrawCube(wall.position, wall.size.x, wall.size.y, wall.size.z, wallColor);
    }

    for (const auto &trigger : triggers_)
    {
        Color c = trigger.active ? GREEN : Fade(RED, 0.4f);
        Vector3 tpos = {trigger.position.x, trigger.position.y, trigger.position.z};
        DrawCubeWires(tpos, trigger.size.x, trigger.size.y, trigger.size.z, c);

        Vector2 textScreenPos = GetWorldToScreen({tpos.x, tpos.y + 1.2f, tpos.z}, camera);
        DrawText("TRIGGER", (int)textScreenPos.x, (int)textScreenPos.y, 20, c);
    }

    for (const auto &inter : interactables_)
    {
        Color c = inter.active ? BLUE : Fade(BLUE, 0.4f);
        Vector3 ipos = {inter.position.x, inter.position.y, inter.position.z};
        DrawCubeWires(ipos, 1.0f, 1.0f, 1.0f, c);
    }

    for (const auto &door : doors_)
    {
        if (!door.isOpen)
            DrawCube(door.position, door.size.x, door.size.y, door.size.z, BROWN);
    }

    EndMode3D();

    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Color{0, 0, 0, 55});
}
