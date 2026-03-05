#ifndef NOVAFORGE_SYSTEMS_TARGETING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_TARGETING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Handles target locking mechanics
 * 
 * Implements Astralis's target locking system where targets must be
 * locked before they can be attacked. Lock time depends on scan resolution.
 */
class TargetingSystem : public ecs::SingleComponentSystem<components::Target> {
public:
    explicit TargetingSystem(ecs::World* world);
    ~TargetingSystem() override = default;
    
    std::string getName() const override { return "TargetingSystem"; }
    
    /**
     * @brief Start locking a target
     * @param entity_id Entity that wants to lock
     * @param target_id Target entity ID
     * @return true if lock started successfully, false otherwise
     */
    bool startLock(const std::string& entity_id, const std::string& target_id);
    
    /**
     * @brief Unlock a target
     * @param entity_id Entity that wants to unlock
     * @param target_id Target entity ID
     */
    void unlockTarget(const std::string& entity_id, const std::string& target_id);
    
    /**
     * @brief Check if target is locked
     * @param entity_id Entity to check
     * @param target_id Target entity ID
     * @return true if target is locked, false otherwise
     */
    bool isTargetLocked(const std::string& entity_id, const std::string& target_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::Target& target_comp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_TARGETING_SYSTEM_H
