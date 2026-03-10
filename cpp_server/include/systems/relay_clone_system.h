#ifndef NOVAFORGE_SYSTEMS_RELAY_CLONE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_RELAY_CLONE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Relay (jump) clone management system
 *
 * Allows characters to install clones at stations and jump between
 * them with a cooldown.  Manages implant sets per clone and enforces
 * the maximum clone count (driven by skill level).
 */
class RelayCloneSystem : public ecs::SingleComponentSystem<components::RelayCloneState> {
public:
    explicit RelayCloneSystem(ecs::World* world);
    ~RelayCloneSystem() override = default;

    std::string getName() const override { return "RelayCloneSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& character_id = "");
    bool installClone(const std::string& entity_id, const std::string& clone_id,
                      const std::string& station_id, const std::string& station_name = "");
    bool destroyClone(const std::string& entity_id, const std::string& clone_id);
    bool jumpToClone(const std::string& entity_id, const std::string& clone_id);
    bool addImplant(const std::string& entity_id, const std::string& clone_id,
                    const std::string& implant_id);
    bool removeImplant(const std::string& entity_id, const std::string& clone_id,
                       const std::string& implant_id);
    int  getCloneCount(const std::string& entity_id) const;
    int  getTotalJumps(const std::string& entity_id) const;
    float getCooldownRemaining(const std::string& entity_id) const;
    bool isOnCooldown(const std::string& entity_id) const;
    bool setMaxClones(const std::string& entity_id, int max_clones);
    int  getMaxClones(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::RelayCloneState& state,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_RELAY_CLONE_SYSTEM_H
