#ifndef NOVAFORGE_SYSTEMS_DRIFTER_AI_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DRIFTER_AI_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Drifter / Triglavian NPC artificial intelligence system
 *
 * Manages advanced Drifter behaviour.  Drifter beam weapons ramp
 * damage over time while firing at the same target.  When provoked
 * past a threshold the Drifter group deploys an area-denial field
 * and calls reinforcement waves from an off-grid carrier.
 */
class DrifterAISystem : public ecs::SingleComponentSystem<components::DrifterAIState> {
public:
    explicit DrifterAISystem(ecs::World* world);
    ~DrifterAISystem() override = default;

    std::string getName() const override { return "DrifterAISystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& site_id = "");
    bool addUnit(const std::string& entity_id, const std::string& unit_id,
                 components::DrifterAIState::DrifterRole role =
                     components::DrifterAIState::DrifterRole::Cruiser,
                 float hp = 2000.0f, float base_dps = 200.0f);
    bool removeUnit(const std::string& entity_id, const std::string& unit_id);
    bool applyDamage(const std::string& entity_id, const std::string& unit_id,
                     float amount);
    bool setThreatLevel(const std::string& entity_id,
                        components::DrifterAIState::ThreatLevel level);
    bool activateAreaDenial(const std::string& entity_id);

    int  getUnitCount(const std::string& entity_id) const;
    int  getAliveCount(const std::string& entity_id) const;
    int  getReinforcementWave(const std::string& entity_id) const;
    float getDamageTaken(const std::string& entity_id) const;
    int  getTotalKills(const std::string& entity_id) const;
    int  getTotalLosses(const std::string& entity_id) const;
    bool isAreaDenialActive(const std::string& entity_id) const;
    components::DrifterAIState::ThreatLevel getThreatLevel(
        const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DrifterAIState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DRIFTER_AI_SYSTEM_H
