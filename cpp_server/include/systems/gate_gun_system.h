#ifndef NOVAFORGE_SYSTEMS_GATE_GUN_SYSTEM_H
#define NOVAFORGE_SYSTEMS_GATE_GUN_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Automated gate gun engagement system
 *
 * Manages sentry guns at stargates.  Gate guns lock onto criminal
 * targets within range and deal timed damage with falloff.
 */
class GateGunSystem : public ecs::SingleComponentSystem<components::GateGunState> {
public:
    explicit GateGunSystem(ecs::World* world);
    ~GateGunSystem() override = default;

    std::string getName() const override { return "GateGunSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& gate_id = "");
    bool addTarget(const std::string& entity_id, const std::string& target_id,
                   float threat_level = 1.0f, bool is_criminal = true);
    bool removeTarget(const std::string& entity_id, const std::string& target_id);
    int  getTargetCount(const std::string& entity_id) const;
    int  getTotalShotsFired(const std::string& entity_id) const;
    float getTotalDamageDealt(const std::string& entity_id) const;
    int  getTotalKills(const std::string& entity_id) const;
    bool setOnline(const std::string& entity_id, bool online);
    bool isOnline(const std::string& entity_id) const;
    float getDamageAtRange(const std::string& entity_id, float range) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::GateGunState& gun,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_GATE_GUN_SYSTEM_H
