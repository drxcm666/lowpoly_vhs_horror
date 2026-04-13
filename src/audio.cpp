#include "audio.hpp"

#include <sstream>
#include <cmath>
#include <algorithm>

void AudioManager::loadFromManifest(const std::string &path)
{
    clearAll();

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
    {
        SetSoundVolume(it->second, 0.35f);
        PlaySound(it->second);
    }
}

void AudioManager::playMusic(const std::string &name)
{
    auto it = musicMap_.find(name);
    if (it != musicMap_.end())
    {
        SetMusicVolume(it->second, 0.8f);
        if (!IsMusicStreamPlaying(it->second))
            PlayMusicStream(it->second);
    }
}

void AudioManager::playMusic(const std::string &name,
                             Vector3 position, float minDistance, float maxDistance, const std::string &zone)
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

    float volume = 0.0f;
    if (distance > maxDistance)
        volume = 0.0f;
    else if (distance < minDistance)
        volume = 0.8f;
    else
    {
        volume = 1.0f - ((distance - minDistance) / (maxDistance - minDistance));
        volume = std::clamp(volume, 0.0f, 1.0f);
    }

    if (!zone.empty() && zone != currentZone_)
        volume = 0.0f;

    SetMusicVolume(music, volume);

    if (!IsMusicStreamPlaying(music))
        PlayMusicStream(music);
}

void AudioManager::stopMusic(const std::string &name)
{
    auto it = musicMap_.find(name);
    if (it == musicMap_.end())
        return;

    if (IsMusicStreamPlaying(it->second))
        StopMusicStream(it->second);
}

void AudioManager::muteMusic(const std::string &name)
{
    auto it = musicMap_.find(name);
    if (it == musicMap_.end())
        return;

    SetMusicVolume(it->second, 0.0f);

    if (!IsMusicStreamPlaying(it->second))
        PlayMusicStream(it->second);
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

        try
        {
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

                if (wordIndex == 9)
                    object.zone = word;

                if (wordIndex >= 4)
                    object.position = Vector3{tempX, tempY, tempZ};

                wordIndex++;
            }
        }
        catch (const std::exception &e)
        {
            TraceLog(LOG_WARNING, "Bad emitter line");
            continue;
        }

        if (!object.name.empty() && (wordIndex == 10))
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

void AudioManager::processTimers(const float dt)
{
    for (auto &pair : emittersMap_)
    {
        std::string name = pair.second.name;
        std::string pref;
        if (name.length() >= 4)
            pref = name.substr(0, 4);

        if (pair.second.maxDelay <= 0.0f)
            continue;

        pair.second.timer -= dt;
        if (pair.second.timer <= 0)
        {
            pair.second.timer = static_cast<float>(
                    GetRandomValue(pair.second.minDelay, pair.second.maxDelay));
            if (pref == "sfx_")
            {
                
                auto sfx_idx = soundMap_.find(name);
                if (sfx_idx == soundMap_.end())
                    continue;

                playSound(name);
            }
            else if (pref == "mus_")
            {
                if (pair.second.position.has_value())
                    playMusic(name,
                              pair.second.position.value(),
                              pair.second.minDistance,
                              pair.second.maxDistance,
                              pair.second.zone);
                else
                    playMusic(name);
            }
        }
    }
}

void AudioManager::updatePositionalMusic()
{
    for (auto &pair : emittersMap_)
    {
        std::string name = pair.second.name;
        std::string pref;
        if (name.length() >= 4)
            pref = name.substr(0, 4);
        if (pref == "mus_" && pair.second.position.has_value())
        {
            auto musicIt = musicMap_.find(name);
            if (musicIt != musicMap_.end() && IsMusicStreamPlaying(musicIt->second))
            {
                playMusic(name,
                            pair.second.position.value(),
                            pair.second.minDistance,
                            pair.second.maxDistance,
                            pair.second.zone);
            }
        }
    }
}

void AudioManager::playEmitter(const std::string &name)
{
    auto it = emittersMap_.find(name);
    if (it == emittersMap_.end())
        return;

    if (it->second.maxDelay > 0.0f)
        return;

    if (it->second.position.has_value())
    {
        playMusic(name,
                  it->second.position.value(),
                  it->second.minDistance,
                  it->second.maxDistance,
                  it->second.zone);
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

    // processTimeEmitters(dt);
    processTimers(dt);
    updatePositionalMusic();
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
    emittersMap_.clear();
}

AudioManager::~AudioManager()
{
    clearAll();
}