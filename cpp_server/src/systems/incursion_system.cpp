#include "systems/incursion_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using Inc = components::Incursion;
using Wave = components::Incursion::Wave;

Wave* findWave(Inc* inc, int wave_number) {
    for (auto& w : inc->waves) {
        if (w.wave_number == wave_number) return &w;
    }
    return nullptr;
}

const Wave* findWaveConst(const Inc* inc, int wave_number) {
    for (const auto& w : inc->waves) {
        if (w.wave_number == wave_number) return &w;
    }
    return nullptr;
}

} // anonymous namespace

IncursionSystem::IncursionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void IncursionSystem::updateComponent(ecs::Entity& entity, components::Incursion& inc, float delta_time) {
    if (!inc.active) return;

    inc.elapsed += delta_time;

    if (inc.state == Inc::IncursionState::Active) {
        bool all_defeated = !inc.waves.empty();
        for (const auto& w : inc.waves) {
            if (!w.defeated) { all_defeated = false; break; }
        }
        if (all_defeated) {
            inc.state = Inc::IncursionState::Withdrawing;
            return;
        }
    }

    if (inc.state == Inc::IncursionState::Withdrawing && inc.influence <= 0.0f) {
        inc.state = Inc::IncursionState::Defeated;
    }
}

bool IncursionSystem::initialize(const std::string& entity_id,
    const std::string& incursion_id, const std::string& system_id, int tier) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::Incursion>();
    comp->incursion_id = incursion_id;
    comp->system_id = system_id;
    comp->tier = tier;
    entity->addComponent(std::move(comp));
    return true;
}

bool IncursionSystem::addWave(const std::string& entity_id, int wave_number,
    const std::string& npc_type, int count) {
    auto* inc = getComponentFor(entity_id);
    if (!inc) return false;
    if (static_cast<int>(inc->waves.size()) >= inc->max_waves) return false;
    if (findWave(inc, wave_number)) return false;

    Wave w;
    w.wave_number = wave_number;
    w.npc_type = npc_type;
    w.count = count;
    inc->waves.push_back(w);
    return true;
}

bool IncursionSystem::spawnWave(const std::string& entity_id, int wave_number) {
    auto* inc = getComponentFor(entity_id);
    if (!inc) return false;

    auto* w = findWave(inc, wave_number);
    if (!w || w->spawned) return false;
    w->spawned = true;
    inc->total_waves_spawned++;
    return true;
}

bool IncursionSystem::defeatWave(const std::string& entity_id, int wave_number) {
    auto* inc = getComponentFor(entity_id);
    if (!inc) return false;

    auto* w = findWave(inc, wave_number);
    if (!w || w->defeated) return false;
    w->defeated = true;
    inc->total_waves_defeated++;
    if (inc->max_waves > 0) {
        inc->influence -= 1.0f / static_cast<float>(inc->max_waves);
        if (inc->influence < 0.0f) inc->influence = 0.0f;
    }
    return true;
}

bool IncursionSystem::joinIncursion(const std::string& entity_id,
    const std::string& participant_id) {
    auto* inc = getComponentFor(entity_id);
    if (!inc) return false;
    if (static_cast<int>(inc->participants.size()) >= inc->max_participants) return false;

    for (const auto& p : inc->participants) {
        if (p == participant_id) return false;
    }
    inc->participants.push_back(participant_id);
    return true;
}

bool IncursionSystem::leaveIncursion(const std::string& entity_id,
    const std::string& participant_id) {
    auto* inc = getComponentFor(entity_id);
    if (!inc) return false;

    auto it = std::find(inc->participants.begin(), inc->participants.end(), participant_id);
    if (it == inc->participants.end()) return false;
    inc->participants.erase(it);
    return true;
}

bool IncursionSystem::contributeRewards(const std::string& entity_id, float amount) {
    auto* inc = getComponentFor(entity_id);
    if (!inc) return false;
    inc->reward_pool += amount;
    return true;
}

float IncursionSystem::getInfluence(const std::string& entity_id) const {
    auto* inc = getComponentFor(entity_id);
    return inc ? inc->influence : 0.0f;
}

int IncursionSystem::getParticipantCount(const std::string& entity_id) const {
    auto* inc = getComponentFor(entity_id);
    return inc ? static_cast<int>(inc->participants.size()) : 0;
}

int IncursionSystem::getState(const std::string& entity_id) const {
    auto* inc = getComponentFor(entity_id);
    return inc ? static_cast<int>(inc->state) : 0;
}

float IncursionSystem::getRewardPool(const std::string& entity_id) const {
    auto* inc = getComponentFor(entity_id);
    return inc ? inc->reward_pool : 0.0f;
}

int IncursionSystem::getTotalWavesDefeated(const std::string& entity_id) const {
    auto* inc = getComponentFor(entity_id);
    return inc ? inc->total_waves_defeated : 0;
}

} // namespace systems
} // namespace atlas
