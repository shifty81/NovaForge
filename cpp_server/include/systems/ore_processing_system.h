#ifndef NOVAFORGE_SYSTEMS_ORE_PROCESSING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ORE_PROCESSING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Converts raw mined ore into refined materials
 *
 * Bridges the mine → trade loop in the vertical slice.  When a mining
 * laser cycle completes, raw ore is queued here.  Each tick, active jobs
 * advance; on completion the refined output is credited to the entity's
 * cargo manifest at the configured efficiency rate.
 */
class OreProcessingSystem : public ecs::SingleComponentSystem<components::OreProcessing> {
public:
    explicit OreProcessingSystem(ecs::World* world);
    ~OreProcessingSystem() override = default;

    std::string getName() const override { return "OreProcessingSystem"; }

    /**
     * @brief Initialize processing state for an entity
     */
    bool initializeProcessing(const std::string& entity_id,
                              float efficiency = 0.75f,
                              int max_jobs = 2);

    /**
     * @brief Queue a new ore batch for processing
     * @return true if job was queued successfully
     */
    bool queueOre(const std::string& entity_id,
                  const std::string& ore_type,
                  float amount,
                  float processing_time = 30.0f);

    /**
     * @brief Get the number of active (incomplete) jobs
     */
    int getActiveJobCount(const std::string& entity_id) const;

    /**
     * @brief Get total refined output since initialization
     */
    float getTotalRefined(const std::string& entity_id) const;

    /**
     * @brief Get total batches completed
     */
    int getBatchesCompleted(const std::string& entity_id) const;

    /**
     * @brief Set processing efficiency (0.0 to 1.0)
     */
    bool setEfficiency(const std::string& entity_id, float efficiency);

    /**
     * @brief Set processing speed multiplier
     */
    bool setProcessingSpeed(const std::string& entity_id, float speed);

protected:
    void updateComponent(ecs::Entity& entity,
                         components::OreProcessing& proc,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ORE_PROCESSING_SYSTEM_H
