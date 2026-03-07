#ifndef NOVAFORGE_SYSTEMS_NPC_SPAWN_SCHEDULER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_SPAWN_SCHEDULER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/npc_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Scheduled NPC spawning — wave management, spawn timers, population caps
 *
 * Controls NPC spawn schedules for asteroid belts, gate camps, and mission
 * pockets.  Each spawn zone has a population cap, a respawn interval, and
 * a wave table that cycles through NPC types.  When the live count drops
 * below cap, the system queues the next wave after the respawn timer
 * elapses.  Supports pausing/resuming and tracks total NPCs spawned.
 */
class NpcSpawnSchedulerSystem : public ecs::SingleComponentSystem<components::NpcSpawnSchedule> {
public:
    explicit NpcSpawnSchedulerSystem(ecs::World* world);
    ~NpcSpawnSchedulerSystem() override = default;

    std::string getName() const override { return "NpcSpawnSchedulerSystem"; }

public:
    bool initialize(const std::string& entity_id, int population_cap, float respawn_interval);
    bool addWaveEntry(const std::string& entity_id, const std::string& npc_type, int count);
    bool removeWaveEntry(const std::string& entity_id, const std::string& npc_type);
    bool recordKill(const std::string& entity_id);
    bool pause(const std::string& entity_id);
    bool resume(const std::string& entity_id);

    int getLiveCount(const std::string& entity_id) const;
    int getPopulationCap(const std::string& entity_id) const;
    int getWaveEntryCount(const std::string& entity_id) const;
    int getCurrentWaveIndex(const std::string& entity_id) const;
    int getTotalSpawned(const std::string& entity_id) const;
    int getTotalKilled(const std::string& entity_id) const;
    bool isPaused(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NpcSpawnSchedule& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_SPAWN_SCHEDULER_SYSTEM_H
