#include "audio.hpp"

#include <sstream>
#include <cmath>
#include <algorithm>

void AudioManager::loadFromManifest(const std::string &path)
{
    char *audioManifest = LoadFileText(path.c_str());
    if (audioManifest == nullptr)
    {
        TraceLog(LOG_ERROR, "Audio file not found");
        return;
    }

    std::stringstream rowStream(audioManifest);
    std::string line;
    while (std::getline(rowStream, line, '\n'))
    {
        if (line.empty() || (line.substr(0, 2) == "//"))
            continue;

        std::stringstream wordStream(line);
        std::string word;
        int wordIndex = 0;
        AudioFile object;
        while (wordStream >> word)
        {
            if ((wordIndex == 0))
                object.type = word;

            if ((wordIndex == 1))
                object.name = word;

            if ((wordIndex == 2))
                object.path = word;

            wordIndex++;
        }

        if (object.type == "Sound")
            soundMap_[object.name] = LoadSound(object.path.c_str());

        if (object.type == "Music")
        {
            musicMap_[object.name] = LoadMusicStream(object.path.c_str());
            musicMap_[object.name].looping = true;
        }
    }

    UnloadFileText(audioManifest);
}

void AudioManager::playSound(const std::string &name)
{
    auto it = soundMap_.find(name);
    if (it != soundMap_.end())
        PlaySound(it->second);
}

void AudioManager::playMusic(const std::string &name)
{
    auto it = musicMap_.find(name);
    if (it != musicMap_.end())
    {
        if (!IsMusicStreamPlaying(it->second))
            PlayMusicStream(it->second);
    }
}

void AudioManager::playMusic(const std::string &name,
                             Vector3 position, float minDistance, float maxDistance)
{
    auto it = musicMap_.find(name);
    if (it == musicMap_.end())
        return;

    if (maxDistance <= minDistance)
        maxDistance = minDistance + 0.001f;

    Music &music = it->second;

    Vector3 diff = {listenerPosition_.x - position.x,
                    listenerPosition_.y - position.y,
                    listenerPosition_.z - position.z};

    float distance = std::sqrt(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z);

    if (distance > maxDistance)
        SetMusicVolume(music, 0.0f);
    else if (distance < minDistance)
        SetMusicVolume(music, 0.8f);
    else
    {
        float volume = 1.0f - ((distance - minDistance) / (maxDistance - minDistance));
        volume = std::clamp(volume, 0.0f, 1.0f);
        SetMusicVolume(music, volume);
    }

    if (!IsMusicStreamPlaying(music))
        PlayMusicStream(music);
}

void AudioManager::stopMusic(const std::string &name)
{
    auto it = musicMap_.find(name);
    if (it == musicMap_.end())
        return;

    if (IsMusicStreamPlaying(it->second))
        SetMusicVolume(it->second, 0.0f);
}

void AudioManager::parseEmitters(const std::string &path)
{
    emittersMap_.clear();

    char *emittersText = LoadFileText(path.c_str());
    if (emittersText == nullptr)
    {
        TraceLog(LOG_ERROR, "Emitters file not found");
        return;
    }

    std::stringstream rowStream(emittersText);
    std::string line;
    while (std::getline(rowStream, line, '\n'))
    {
        if (line.empty() || line.substr(0, 2) == "//")
            continue;

        std::stringstream wordStream(line);
        std::string word;
        float tempX = 0.0f, tempY = 0.0f, tempZ = 0.0f;
        int wordIndex{0};
        AudioEmitter object;
        while (wordStream >> word)
        {
            if (wordIndex == 0)
                object.name = word;

            if (wordIndex == 1)
                tempX = std::stof(word);

            if (wordIndex == 2)
                tempY = std::stof(word);

            if (wordIndex == 3)
                tempZ = std::stof(word);

            if (wordIndex == 4)
                object.minDistance = std::stof(word);

            if (wordIndex == 5)
                object.maxDistance = std::stof(word);

            if (wordIndex == 6)
                object.timer = std::stof(word);

            if (wordIndex == 7)
                object.minDelay = std::stof(word);

            if (wordIndex == 8)
                object.maxDelay = std::stof(word);
                
            if (wordIndex >= 4)
                object.position = Vector3{tempX, tempY, tempZ};
            
            
            wordIndex++;
        }

        if (!object.name.empty() && (wordIndex >= 6))
            emittersMap_[object.name] = object;
    }

    for (const auto &pair : emittersMap_)
    {
        if (pair.second.timer > 0.0f)
        {
            auto musicIt = musicMap_.find(pair.first);
            if (musicIt != musicMap_.end()) 
                musicIt->second.looping = false;
        }
    }

    UnloadFileText(emittersText);
}

void AudioManager::processTimeEmitters(const float dt)
{
    for (auto &pair : emittersMap_)
    {
        if (pair.second.timer <= 0.0f)
            continue;
        
        std::string name = pair.second.name;
        std::string pref;
        if (name.length() >= 4)
            pref = name.substr(0, 4);

        pair.second.timer -= dt;
        if (pair.second.timer <= 0)
        {
            if (pref == "sfx_")
            {
                pair.second.timer = static_cast<float>(GetRandomValue(pair.second.minDelay, pair.second.maxDelay));
                auto sfx_idx = soundMap_.find(name);
                if (sfx_idx == soundMap_.end())
                    return;

                playSound(name);
            }
            else if (pref == "mus_")
            {
                pair.second.timer = static_cast<float>(GetRandomValue(pair.second.minDelay, pair.second.maxDelay));
                if (pair.second.position.has_value())
                    playMusic(name, pair.second.position.value(), 
                              pair.second.minDistance, pair.second.maxDistance);
                else
                    playMusic(name);
            }
        }
    }
}

void AudioManager::playEmitter(const std::string &name)
{
    auto it = emittersMap_.find(name);
    if (it == emittersMap_.end())
        return;

    if (it->second.position.has_value())
    {
        playMusic(name, it->second.position.value(), 
              emittersMap_[name].minDistance, emittersMap_[name].maxDistance);
    }
    else
        playMusic(name);

}

void AudioManager::update(float dt)
{
    for (const auto &pair : musicMap_)
    {
        if (IsMusicStreamPlaying(pair.second))
            UpdateMusicStream(pair.second);
    }

    processTimeEmitters(dt);
}

void AudioManager::clearAll()
{
    for (const auto &pair : soundMap_)
        UnloadSound(pair.second);

    for (const auto &pair : musicMap_)
    {
        if (IsMusicStreamPlaying(pair.second))
            StopMusicStream(pair.second);

        UnloadMusicStream(pair.second);
    }

    soundMap_.clear();
    musicMap_.clear();
}

AudioManager::~AudioManager()
{
    clearAll();
}