#ifndef NOVAFORGE_SYSTEMS_ECM_JAMMING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ECM_JAMMING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief ECM electronic-countermeasures jamming system
 *
 * Models EVE Online ECM mechanics.  Each cycle a jammer randomly attempts to
 * break the target's sensor lock.  Jam success probability is
 * (jam_strength / sensor_strength); success sets is_jammed = true for the
 * next cycle.  Multiple jammers from different sources stack additively.
 * While jammed the entity cannot initiate or maintain target locks.
 */
class EcmJammingSystem
    : public ecs::SingleComponentSystem<components::EcmJammingState> {
public:
    explicit EcmJammingSystem(ecs::World* world);
    ~EcmJammingSystem() override = default;

    std::string getName() const override { return "EcmJammingSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id,
                    float sensor_strength = 10.0f);

    // --- Jammer management ---
    bool applyJammer(const std::string& entity_id,
                     const std::string& source_id,
                     float jam_strength,
                     float cycle_time = 5.0f);
    bool removeJammer(const std::string& entity_id,
                      const std::string& source_id);
    bool clearJammers(const std::string& entity_id);

    // --- Sensor strength ---
    bool setSensorStrength(const std::string& entity_id, float strength);

    // --- Queries ---
    bool  isJammed(const std::string& entity_id) const;
    int   getJammerCount(const std::string& entity_id) const;
    float getSensorStrength(const std::string& entity_id) const;
    float getTotalJamStrength(const std::string& entity_id) const;
    int   getTotalJamsApplied(const std::string& entity_id) const;
    int   getTotalJamAttempts(const std::string& entity_id) const;
    int   getTotalLockBreaks(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::EcmJammingState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ECM_JAMMING_SYSTEM_H
