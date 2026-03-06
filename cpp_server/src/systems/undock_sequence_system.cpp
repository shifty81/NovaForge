#include "systems/undock_sequence_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

UndockSequenceSystem::UndockSequenceSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void UndockSequenceSystem::updateComponent(ecs::Entity& /*entity*/,
    components::UndockSequence& seq, float delta_time) {
    if (!seq.active) return;

    // Phase progression
    if (seq.phase > components::UndockSequence::Docked &&
        seq.phase < components::UndockSequence::Complete) {
        seq.phase_progress += seq.phase_speed * delta_time;
        seq.total_undock_time += delta_time;
        seq.undock_timer += delta_time;

        if (seq.phase_progress >= 1.0f) {
            seq.phase_progress = 0.0f;
            int next = static_cast<int>(seq.phase) + 1;
            seq.phase = static_cast<components::UndockSequence::UndockPhase>(next);
            if (seq.phase == components::UndockSequence::Complete) {
                seq.is_invulnerable = true;
                seq.invulnerability_timer = 0.0f;
            }
        }
    }

    // Invulnerability countdown after undock complete
    if (seq.phase == components::UndockSequence::Complete && seq.is_invulnerable) {
        seq.invulnerability_timer += delta_time;
        if (seq.invulnerability_timer >= seq.invulnerability_duration) {
            seq.is_invulnerable = false;
        }
    }
}

bool UndockSequenceSystem::requestUndock(const std::string& entity_id,
    const std::string& station_id) {
    auto* seq = getComponentFor(entity_id);
    if (!seq) return false;
    if (seq->phase != components::UndockSequence::Docked) return false;
    seq->phase = components::UndockSequence::RequestingUndock;
    seq->station_id = station_id;
    seq->phase_progress = 0.0f;
    seq->total_undock_time = 0.0f;
    seq->undock_timer = 0.0f;
    seq->undock_count++;
    return true;
}

bool UndockSequenceSystem::cancelUndock(const std::string& entity_id) {
    auto* seq = getComponentFor(entity_id);
    if (!seq) return false;
    if (seq->phase != components::UndockSequence::RequestingUndock &&
        seq->phase != components::UndockSequence::HangarExit) return false;
    seq->phase = components::UndockSequence::Docked;
    seq->phase_progress = 0.0f;
    seq->total_undock_time = 0.0f;
    seq->undock_timer = 0.0f;
    return true;
}

int UndockSequenceSystem::getPhase(const std::string& entity_id) const {
    auto* seq = getComponentFor(entity_id);
    return seq ? static_cast<int>(seq->phase) : 0;
}

float UndockSequenceSystem::getProgress(const std::string& entity_id) const {
    auto* seq = getComponentFor(entity_id);
    return seq ? seq->phase_progress : 0.0f;
}

bool UndockSequenceSystem::isUndocking(const std::string& entity_id) const {
    auto* seq = getComponentFor(entity_id);
    if (!seq) return false;
    return seq->phase != components::UndockSequence::Docked &&
           seq->phase != components::UndockSequence::Complete;
}

bool UndockSequenceSystem::isInvulnerable(const std::string& entity_id) const {
    auto* seq = getComponentFor(entity_id);
    return seq ? seq->is_invulnerable : false;
}

float UndockSequenceSystem::getTotalUndockTime(const std::string& entity_id) const {
    auto* seq = getComponentFor(entity_id);
    return seq ? seq->total_undock_time : 0.0f;
}

int UndockSequenceSystem::getUndockCount(const std::string& entity_id) const {
    auto* seq = getComponentFor(entity_id);
    return seq ? seq->undock_count : 0;
}

bool UndockSequenceSystem::setExitPosition(const std::string& entity_id,
    float x, float y, float z) {
    auto* seq = getComponentFor(entity_id);
    if (!seq) return false;
    seq->exit_x = x;
    seq->exit_y = y;
    seq->exit_z = z;
    return true;
}

bool UndockSequenceSystem::setPhaseSpeed(const std::string& entity_id, float speed) {
    if (speed <= 0.0f) return false;
    auto* seq = getComponentFor(entity_id);
    if (!seq) return false;
    seq->phase_speed = speed;
    return true;
}

} // namespace systems
} // namespace atlas
