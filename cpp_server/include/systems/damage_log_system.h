#ifndef NOVAFORGE_SYSTEMS_DAMAGE_LOG_SYSTEM_H
#define NOVAFORGE_SYSTEMS_DAMAGE_LOG_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/combat_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Per-entity hit-by-hit damage event log system
 *
 * Records every individual damage hit applied by or to an entity.  Each entry
 * stores the attacker, defender, damage type (EM/Thermal/Kinetic/Explosive),
 * amount, weapon used, and a hit/miss flag.  Entries are capped at max_entries
 * with oldest-entry eviction so memory stays bounded.
 *
 * Outgoing entries (logOutgoing) record damage dealt by this entity.
 * Incoming entries (logIncoming) record damage received.  Separate running
 * totals are kept for fast aggregation.
 */
class DamageLogSystem
    : public ecs::SingleComponentSystem<components::DamageLog> {
public:
    explicit DamageLogSystem(ecs::World* world);
    ~DamageLogSystem() override = default;

    std::string getName() const override { return "DamageLogSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Logging ---
    bool logOutgoing(const std::string& entity_id,
                     const std::string& defender_id,
                     components::DamageLog::DamageType damage_type,
                     float amount,
                     const std::string& weapon,
                     bool hit = true);

    bool logIncoming(const std::string& entity_id,
                     const std::string& attacker_id,
                     components::DamageLog::DamageType damage_type,
                     float amount,
                     const std::string& weapon,
                     bool hit = true);

    // --- Management ---
    bool clearEntries(const std::string& entity_id);

    // --- Queries ---
    int   getEntryCount(const std::string& entity_id) const;
    float getTotalOutgoing(const std::string& entity_id) const;
    float getTotalIncoming(const std::string& entity_id) const;
    int   getTotalMisses(const std::string& entity_id) const;
    int   getTotalShots(const std::string& entity_id) const;
    components::DamageLog::DamageEntry
          getMostRecentEntry(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::DamageLog& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_DAMAGE_LOG_SYSTEM_H
