#include "scene.hpp"

void Scene::loadEnvironment()
{
    for (const auto &i : models_)
        UnloadModel(i.model);

    models_.clear();
    lightSources_.clear();
    baseLightColors_.clear();

    if (lightShader_.id != 0)
        UnloadShader(lightShader_);

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
        {0.6f, 0.6f, 0.6f},
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

    ModelInstance monsterSecond{
        "monsterSecond",
        LoadModel("assets/monster/monster_2.glb"),
        {13.46f, -100.0f, 12.3f},
        {1.0f, 0.0f, 0.0f},
        90.0f,
        {0.006f, 0.006f, 0.006f},
        nightTint,
        false,
        true,
        {13.46f, -100.0f, 12.3f},
        90.0f,
        nightTint};
    for (int i = 0; i < monsterSecond.model.materialCount; i++)
    {
        monsterSecond.model.materials[i].shader = lightShader_;
    }
    models_.push_back(monsterSecond);

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