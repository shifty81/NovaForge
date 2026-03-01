#include "AudioEngine.h"
#include <algorithm>

namespace atlas::audio {

void AudioEngine::Init() {
    m_sounds.clear();
    m_nextId = 1;
    m_masterVolume = 1.0f;
    m_initialized = true;
}

void AudioEngine::Shutdown() {
    for (auto& [id, source] : m_sounds) {
        source.state = SoundState::Stopped;
    }
    m_sounds.clear();
    m_initialized = false;
}

SoundID AudioEngine::LoadSound(const std::string& name) {
    SoundID id = m_nextId++;
    SoundSource source;
    source.id = id;
    source.name = name;
    m_sounds[id] = source;
    return id;
}

void AudioEngine::UnloadSound(SoundID id) {
    m_sounds.erase(id);
}

bool AudioEngine::HasSound(SoundID id) const {
    return m_sounds.count(id) > 0;
}

size_t AudioEngine::SoundCount() const {
    return m_sounds.size();
}

void AudioEngine::Play(SoundID id) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        it->second.state = SoundState::Playing;
    }
}

void AudioEngine::Pause(SoundID id) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end() && it->second.state == SoundState::Playing) {
        it->second.state = SoundState::Paused;
    }
}

void AudioEngine::Stop(SoundID id) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        it->second.state = SoundState::Stopped;
    }
}

SoundState AudioEngine::GetState(SoundID id) const {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        return it->second.state;
    }
    return SoundState::Stopped;
}

void AudioEngine::SetVolume(SoundID id, float volume) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        it->second.volume = std::clamp(volume, 0.0f, 1.0f);
    }
}

float AudioEngine::GetVolume(SoundID id) const {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        return it->second.volume;
    }
    return 0.0f;
}

void AudioEngine::SetPitch(SoundID id, float pitch) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        it->second.pitch = std::max(0.1f, pitch);
    }
}

float AudioEngine::GetPitch(SoundID id) const {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        return it->second.pitch;
    }
    return 1.0f;
}

void AudioEngine::SetLooping(SoundID id, bool looping) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        it->second.looping = looping;
    }
}

bool AudioEngine::IsLooping(SoundID id) const {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        return it->second.looping;
    }
    return false;
}

void AudioEngine::SetPosition(SoundID id, float x, float y, float z) {
    auto it = m_sounds.find(id);
    if (it != m_sounds.end()) {
        it->second.posX = x;
        it->second.posY = y;
        it->second.posZ = z;
    }
}

void AudioEngine::SetMasterVolume(float volume) {
    m_masterVolume = std::clamp(volume, 0.0f, 1.0f);
}

float AudioEngine::GetMasterVolume() const {
    return m_masterVolume;
}

void AudioEngine::Update(float dt) {
    if (!m_initialized) return;

    for (auto& [id, source] : m_sounds) {
        if (source.state != SoundState::Playing) continue;

        // Advance playback position
        source.playbackTime += dt * source.pitch;

        // Handle non-looping sounds that exceed their duration
        if (!source.looping && source.duration > 0.0f &&
            source.playbackTime >= source.duration) {
            source.state = SoundState::Stopped;
            source.playbackTime = 0.0f;
        }

        // Wrap looping sounds
        if (source.looping && source.duration > 0.0f &&
            source.playbackTime >= source.duration) {
            source.playbackTime -= source.duration;
        }
    }
}

}
