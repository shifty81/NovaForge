#include "systems/npc_spawn_scheduler_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

NpcSpawnSchedulerSystem::NpcSpawnSchedulerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NpcSpawnSchedulerSystem::updateComponent(ecs::Entity& entity,
    components::NpcSpawnSchedule& comp, float delta_time) {
    if (!comp.active) return;
    if (comp.paused) return;
    comp.elapsed += delta_time;

    // If live count is below cap and we have wave entries, advance respawn timer
    if (comp.live_count < comp.population_cap && !comp.wave_entries.empty()) {
        comp.respawn_timer += delta_time;
        if (comp.respawn_timer >= comp.respawn_interval) {
            comp.respawn_timer = 0.0f;

            // Spawn next wave entry
            auto& entry = comp.wave_entries[comp.current_wave_index];
            int to_spawn = std::min(entry.count,
                                    comp.population_cap - comp.live_count);
            comp.live_count += to_spawn;
            comp.total_spawned += to_spawn;

            // Advance wave index (cycle)
            comp.current_wave_index = (comp.current_wave_index + 1) %
                                      static_cast<int>(comp.wave_entries.size());
        }
    }
}

bool NpcSpawnSchedulerSystem::initialize(const std::string& entity_id,
    int population_cap, float respawn_interval) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (population_cap <= 0 || respawn_interval <= 0.0f) return false;

    auto comp = std::make_unique<components::NpcSpawnSchedule>();
    comp->population_cap = population_cap;
    comp->respawn_interval = respawn_interval;
    entity->addComponent(std::move(comp));
    return true;
}

bool NpcSpawnSchedulerSystem::addWaveEntry(const std::string& entity_id,
    const std::string& npc_type, int count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (count <= 0) return false;

    // No duplicate NPC types
    for (const auto& entry : comp->wave_entries) {
        if (entry.npc_type == npc_type) return false;
    }
    if (static_cast<int>(comp->wave_entries.size()) >= comp->max_wave_entries) return false;

    components::NpcSpawnSchedule::WaveEntry entry;
    entry.npc_type = npc_type;
    entry.count = count;
    comp->wave_entries.push_back(entry);
    return true;
}

bool NpcSpawnSchedulerSystem::removeWaveEntry(const std::string& entity_id,
    const std::string& npc_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->wave_entries.begin(), comp->wave_entries.end(),
        [&npc_type](const components::NpcSpawnSchedule::WaveEntry& e) {
            return e.npc_type == npc_type;
        });
    if (it == comp->wave_entries.end()) return false;
    comp->wave_entries.erase(it);

    // Reset wave index if necessary
    if (!comp->wave_entries.empty()) {
        comp->current_wave_index = comp->current_wave_index %
                                   static_cast<int>(comp->wave_entries.size());
    } else {
        comp->current_wave_index = 0;
    }
    return true;
}

bool NpcSpawnSchedulerSystem::recordKill(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->live_count <= 0) return false;

    comp->live_count--;
    comp->total_killed++;
    return true;
}

bool NpcSpawnSchedulerSystem::pause(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->paused) return false;
    comp->paused = true;
    return true;
}

bool NpcSpawnSchedulerSystem::resume(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (!comp->paused) return false;
    comp->paused = false;
    return true;
}

int NpcSpawnSchedulerSystem::getLiveCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->live_count : 0;
}

int NpcSpawnSchedulerSystem::getPopulationCap(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->population_cap : 0;
}

int NpcSpawnSchedulerSystem::getWaveEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->wave_entries.size()) : 0;
}

int NpcSpawnSchedulerSystem::getCurrentWaveIndex(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_wave_index : 0;
}

int NpcSpawnSchedulerSystem::getTotalSpawned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_spawned : 0;
}

int NpcSpawnSchedulerSystem::getTotalKilled(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_killed : 0;
}

bool NpcSpawnSchedulerSystem::isPaused(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->paused : false;
}

} // namespace systems
} // namespace atlas
