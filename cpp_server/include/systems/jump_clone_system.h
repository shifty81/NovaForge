#ifndef NOVAFORGE_SYSTEMS_JUMP_CLONE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_JUMP_CLONE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Jump clone management system
 *
 * Manages a player's jump clones, which allow instant travel between
 * stations at the cost of a cooldown timer.  Players install clones at
 * stations, optionally add implants, and jump between them.  Jumping
 * triggers a cooldown (default 24 h) that counts down per-tick.  Clones
 * are capped at max_clones (default 10).  Lifetime counters track total
 * jumps, installations, and destructions.
 */
class JumpCloneSystem
    : public ecs::SingleComponentSystem<components::JumpCloneState> {
public:
    explicit JumpCloneSystem(ecs::World* world);
    ~JumpCloneSystem() override = default;

    std::string getName() const override { return "JumpCloneSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Clone management ---
    bool installClone(const std::string& entity_id,
                      const std::string& clone_id,
                      const std::string& station_id,
                      const std::string& station_name);
    bool destroyClone(const std::string& entity_id,
                      const std::string& clone_id);
    bool clearClones(const std::string& entity_id);

    // --- Implant management ---
    bool addImplant(const std::string& entity_id,
                    const std::string& clone_id,
                    const std::string& implant_id,
                    const std::string& implant_name,
                    int slot);
    bool removeImplant(const std::string& entity_id,
                       const std::string& clone_id,
                       const std::string& implant_id);

    // --- Jump operations ---
    bool jumpToClone(const std::string& entity_id,
                     const std::string& clone_id);

    // --- Configuration ---
    bool setMaxClones(const std::string& entity_id, int max_clones);
    bool setCooldownDuration(const std::string& entity_id, float duration);
    bool setActiveStation(const std::string& entity_id,
                          const std::string& station_id);

    // --- Queries ---
    int   getCloneCount(const std::string& entity_id) const;
    bool  hasClone(const std::string& entity_id,
                   const std::string& clone_id) const;
    std::string getCloneStation(const std::string& entity_id,
                                const std::string& clone_id) const;
    std::string getCloneStationName(const std::string& entity_id,
                                    const std::string& clone_id) const;
    int   getImplantCount(const std::string& entity_id,
                          const std::string& clone_id) const;
    bool  hasImplant(const std::string& entity_id,
                     const std::string& clone_id,
                     const std::string& implant_id) const;
    float getCooldownRemaining(const std::string& entity_id) const;
    bool  isOnCooldown(const std::string& entity_id) const;
    std::string getActiveCloneId(const std::string& entity_id) const;
    std::string getActiveStationId(const std::string& entity_id) const;
    int   getTotalJumps(const std::string& entity_id) const;
    int   getTotalClonesInstalled(const std::string& entity_id) const;
    int   getTotalClonesDestroyed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::JumpCloneState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_JUMP_CLONE_SYSTEM_H
