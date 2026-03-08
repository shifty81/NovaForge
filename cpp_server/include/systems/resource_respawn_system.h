#ifndef NOVAFORGE_SYSTEMS_RESOURCE_RESPAWN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RESOURCE_RESPAWN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages respawning of minable resources, exploration sites, and NPC spawns
 *
 * Tracks depletable resources within a zone.  When a resource is marked
 * depleted it enters a cooldown; once the cooldown expires it is flagged
 * as respawned (with an optional yield multiplier variation).  This
 * creates the persistent "living world" loop where belts, sites, and
 * NPC groups regenerate over time.
 */
class ResourceRespawnSystem : public ecs::SingleComponentSystem<components::ResourceRespawnState> {
public:
    explicit ResourceRespawnSystem(ecs::World* world);
    ~ResourceRespawnSystem() override = default;

    std::string getName() const override { return "ResourceRespawnSystem"; }

    bool initialize(const std::string& entity_id, const std::string& zone_id);
    bool addResource(const std::string& entity_id, const std::string& resource_id,
                     const std::string& resource_type, float cooldown);
    bool removeResource(const std::string& entity_id, const std::string& resource_id);
    bool depleteResource(const std::string& entity_id, const std::string& resource_id);
    bool setYieldMultiplier(const std::string& entity_id, const std::string& resource_id,
                            float multiplier);

    int   getResourceCount(const std::string& entity_id) const;
    int   getDepletedCount(const std::string& entity_id) const;
    int   getRespawnedCount(const std::string& entity_id) const;
    int   getTotalRespawns(const std::string& entity_id) const;
    int   getTotalDepletions(const std::string& entity_id) const;
    float getCooldownRemaining(const std::string& entity_id, const std::string& resource_id) const;
    bool  isResourceDepleted(const std::string& entity_id, const std::string& resource_id) const;
    bool  isResourceRespawned(const std::string& entity_id, const std::string& resource_id) const;
    std::string getZoneId(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ResourceRespawnState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RESOURCE_RESPAWN_SYSTEM_H
