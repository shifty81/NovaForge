#ifndef NOVAFORGE_SYSTEMS_PERSISTENCE_DELTA_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PERSISTENCE_DELTA_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks long-term consequences of player actions with magnitude and decay
 *
 * Records player actions by category, applies decay over time, and triggers
 * consequences when accumulated impact exceeds thresholds.
 */
class PersistenceDeltaSystem : public ecs::SingleComponentSystem<components::PersistenceDelta> {
public:
    explicit PersistenceDeltaSystem(ecs::World* world);
    ~PersistenceDeltaSystem() override = default;

    std::string getName() const override { return "PersistenceDeltaSystem"; }

    bool initializeTracker(const std::string& entity_id);
    bool recordAction(const std::string& entity_id, const std::string& action_id,
                      const std::string& category, float magnitude,
                      float decay_rate, bool permanent);
    float getCategoryImpact(const std::string& entity_id, const std::string& category) const;
    float getTotalImpact(const std::string& entity_id) const;
    int getActionCount(const std::string& entity_id) const;
    bool isConsequenceTriggered(const std::string& entity_id) const;
    bool clearConsequence(const std::string& entity_id);
    int getEntryCount(const std::string& entity_id) const;
    float getPositiveImpact(const std::string& entity_id) const;
    float getNegativeImpact(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PersistenceDelta& pd, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PERSISTENCE_DELTA_SYSTEM_H
