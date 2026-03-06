#ifndef NOVAFORGE_SYSTEMS_ENTITY_LIFECYCLE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ENTITY_LIFECYCLE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/core_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Entity lifecycle tracking system
 *
 * Records entity creation, destruction, and state change events.
 * Tracks lifetime duration, death causes, and spawn/destroy metrics
 * for debugging, profiling, and content balance tuning.
 */
class EntityLifecycleSystem : public ecs::SingleComponentSystem<components::EntityLifecycle> {
public:
    explicit EntityLifecycleSystem(ecs::World* world);
    ~EntityLifecycleSystem() override = default;

    std::string getName() const override { return "EntityLifecycleSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& entity_type,
                    const std::string& spawn_source);
    bool recordSpawn(const std::string& entity_id, const std::string& entity_type);
    bool recordDestroy(const std::string& entity_id, int death_cause,
                       const std::string& cause_detail);
    bool recordStateChange(const std::string& entity_id, const std::string& description);
    bool markDead(const std::string& entity_id, int death_cause);
    int getEventCount(const std::string& entity_id) const;
    float getLifetime(const std::string& entity_id) const;
    int getDeathCause(const std::string& entity_id) const;
    int getTotalSpawned(const std::string& entity_id) const;
    int getTotalDestroyed(const std::string& entity_id) const;
    int getTotalStateChanges(const std::string& entity_id) const;
    bool isAlive(const std::string& entity_id) const;
    std::string getEntityType(const std::string& entity_id) const;
    std::string getSpawnSource(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::EntityLifecycle& lc, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ENTITY_LIFECYCLE_SYSTEM_H
