#ifndef NOVAFORGE_SYSTEMS_NPC_FLEET_COMPOSITION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NPC_FLEET_COMPOSITION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fleet_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Defines NPC fleet templates and manages fleet spawning
 *
 * Fleet compositions are created from templates specifying ship types,
 * roles (DPS/Tank/Support/Commander), and difficulty tiers.  The system
 * tracks spawn cooldowns and scales difficulty based on system security.
 */
class NpcFleetCompositionSystem : public ecs::SingleComponentSystem<components::NpcFleetComposition> {
public:
    explicit NpcFleetCompositionSystem(ecs::World* world);
    ~NpcFleetCompositionSystem() override = default;

    std::string getName() const override { return "NpcFleetCompositionSystem"; }

public:
    bool initialize(const std::string& entity_id, const std::string& template_id,
                    const std::string& template_name);
    bool addShip(const std::string& entity_id, const std::string& ship_type,
                 const std::string& role, float threat_value, bool is_commander);
    bool setSecurityLevel(const std::string& entity_id, float security_level);
    bool setDifficulty(const std::string& entity_id, const std::string& difficulty);
    bool setSpawnCooldown(const std::string& entity_id, float cooldown);
    bool requestSpawn(const std::string& entity_id);
    bool recordDestroyed(const std::string& entity_id);

    int getShipCount(const std::string& entity_id) const;
    float getTotalThreat(const std::string& entity_id) const;
    std::string getDifficulty(const std::string& entity_id) const;
    int getFleetsSpawned(const std::string& entity_id) const;
    int getFleetsDestroyed(const std::string& entity_id) const;
    float getCooldownRemaining(const std::string& entity_id) const;
    bool isReady(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::NpcFleetComposition& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NPC_FLEET_COMPOSITION_SYSTEM_H
