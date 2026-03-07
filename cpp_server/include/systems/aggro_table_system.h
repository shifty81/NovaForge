#ifndef NOVAFORGE_SYSTEMS_AGGRO_TABLE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_AGGRO_TABLE_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/npc_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages per-NPC aggro tables for AI targeting decisions
 *
 * Tracks threat per attacker, decays stale entries, and exposes the
 * highest-threat target for the AI system to use in target selection.
 */
class AggroTableSystem : public ecs::SingleComponentSystem<components::AggroTable> {
public:
    explicit AggroTableSystem(ecs::World* world);
    ~AggroTableSystem() override = default;

    std::string getName() const override { return "AggroTableSystem"; }

    bool initializeAggroTable(const std::string& entity_id,
                              float decay_rate = 2.0f, float decay_delay = 5.0f);
    bool recordThreat(const std::string& entity_id,
                      const std::string& attacker_id, float amount);
    float getThreat(const std::string& entity_id,
                    const std::string& attacker_id) const;
    std::string getTopThreat(const std::string& entity_id) const;
    int getEntryCount(const std::string& entity_id) const;
    int getTotalThreatEvents(const std::string& entity_id) const;
    float getTotalThreatAccumulated(const std::string& entity_id) const;
    bool clearTable(const std::string& entity_id);

protected:
    void updateComponent(ecs::Entity& entity, components::AggroTable& at,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_AGGRO_TABLE_SYSTEM_H
