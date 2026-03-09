#ifndef NOVAFORGE_SYSTEMS_ANOMALY_SPAWNING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ANOMALY_SPAWNING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Dynamic anomaly spawning for star systems
 *
 * Periodically spawns combat, gas, relic, data, and wormhole anomalies
 * in a star system.  The number of anomalies scales inversely with
 * security level (more in null-sec).  Completed or expired anomalies
 * are despawned to make room for fresh ones.
 */
class AnomalySpawningSystem : public ecs::SingleComponentSystem<components::AnomalySpawningState> {
public:
    explicit AnomalySpawningSystem(ecs::World* world);
    ~AnomalySpawningSystem() override = default;

    std::string getName() const override { return "AnomalySpawningSystem"; }

    // --- public API ---
    bool initialize(const std::string& entity_id, const std::string& system_id = "",
                    float security_level = 0.5f);
    bool spawnAnomaly(const std::string& entity_id, const std::string& anomaly_id,
                      components::AnomalySpawningState::AnomalyType type =
                          components::AnomalySpawningState::AnomalyType::Combat,
                      int difficulty = 1);
    bool completeAnomaly(const std::string& entity_id, const std::string& anomaly_id);
    bool removeAnomaly(const std::string& entity_id, const std::string& anomaly_id);
    int  getAnomalyCount(const std::string& entity_id) const;
    int  getTotalSpawned(const std::string& entity_id) const;
    int  getTotalCompleted(const std::string& entity_id) const;
    int  getTotalDespawned(const std::string& entity_id) const;
    int  getMaxAnomalies(const std::string& entity_id) const;
    bool setActive(const std::string& entity_id, bool active);
    bool isActive(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::AnomalySpawningState& state,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ANOMALY_SPAWNING_SYSTEM_H
