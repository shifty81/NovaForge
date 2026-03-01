#pragma once
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>

namespace atlas::audio {

using SoundID = uint32_t;

enum class SoundState : uint8_t {
    Stopped,
    Playing,
    Paused
};

struct SoundSource {
    SoundID id = 0;
    std::string name;
    float volume = 1.0f;
    float pitch = 1.0f;
    bool looping = false;
    SoundState state = SoundState::Stopped;
    float posX = 0.0f, posY = 0.0f, posZ = 0.0f;
    float playbackTime = 0.0f;
    float duration = 0.0f;
};

class AudioEngine {
public:
    void Init();
    void Shutdown();

    SoundID LoadSound(const std::string& name);
    void UnloadSound(SoundID id);
    bool HasSound(SoundID id) const;
    size_t SoundCount() const;

    void Play(SoundID id);
    void Pause(SoundID id);
    void Stop(SoundID id);
    SoundState GetState(SoundID id) const;

    void SetVolume(SoundID id, float volume);
    float GetVolume(SoundID id) const;

    void SetPitch(SoundID id, float pitch);
    float GetPitch(SoundID id) const;

    void SetLooping(SoundID id, bool looping);
    bool IsLooping(SoundID id) const;

    void SetPosition(SoundID id, float x, float y, float z);

    void SetMasterVolume(float volume);
    float GetMasterVolume() const;

    void Update(float dt);

private:
    std::unordered_map<SoundID, SoundSource> m_sounds;
    SoundID m_nextId = 1;
    float m_masterVolume = 1.0f;
    bool m_initialized = false;
};

}
