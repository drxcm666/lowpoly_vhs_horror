#pragma once

#include "level_types.hpp"

#include <raylib.h>

#include <unordered_map>
#include <string>

struct AudioFile
{
    std::string type;
    std::string name;
    std::string path;
};

class AudioManager
{
private:
    std::unordered_map<std::string, Sound> soundMap_;
    std::unordered_map<std::string, Music> musicMap_;
    std::unordered_map<std::string, AudioEmitter> emittersMap_;

    Vector3 listenerPosition_{0.0f, 0.0f, 0.0f};

    void processTimeEmitters(const float dt);
    void clearAll();

public:
    void loadFromManifest(const std::string &path);
    void playMusic(const std::string &name);
    void playSound(const std::string &name);
    void playMusic(const std::string &name, Vector3 position, float minDistance, float maxDistance);
    void playEmitter(const std::string &name);
    void update(float dt);
    void parseEmitters(const std::string &path);
    void stopMusic(const std::string &name);


    void setListenerPosition(Vector3 position) { listenerPosition_ = position; }

    auto &getEmitters() { return emittersMap_; }

    AudioManager() = default;
    ~AudioManager();
};
