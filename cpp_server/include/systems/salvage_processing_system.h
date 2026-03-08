#ifndef NOVAFORGE_SYSTEMS_SALVAGE_PROCESSING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SALVAGE_PROCESSING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Wreck salvaging and material extraction system
 *
 * Manages salvage operations on wreckage, tracking processing
 * progress, material yields, and success/failure outcomes.
 * Key for the industry and loot vertical slice.
 */
class SalvageProcessingSystem : public ecs::SingleComponentSystem<components::SalvageProcessingState> {
public:
    explicit SalvageProcessingSystem(ecs::World* world);
    ~SalvageProcessingSystem() override = default;

    std::string getName() const override { return "SalvageProcessingSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id, float processing_speed = 1.0f);

    // Job management
    bool addJob(const std::string& entity_id, const std::string& wreck_id,
                const std::string& material_type, float processing_time,
                float yield_amount, float success_chance);
    bool removeJob(const std::string& entity_id, const std::string& wreck_id);

    // Operations
    bool setProcessingSpeed(const std::string& entity_id, float speed);
    bool setSkillBonus(const std::string& entity_id, float bonus);

    // Queries
    int getJobCount(const std::string& entity_id) const;
    int getActiveJobCount(const std::string& entity_id) const;
    float getJobProgress(const std::string& entity_id, const std::string& wreck_id) const;
    bool isJobCompleted(const std::string& entity_id, const std::string& wreck_id) const;
    bool isJobSuccessful(const std::string& entity_id, const std::string& wreck_id) const;
    float getTotalMaterialsSalvaged(const std::string& entity_id) const;
    int getTotalJobsCompleted(const std::string& entity_id) const;
    int getTotalJobsFailed(const std::string& entity_id) const;
    float getProcessingSpeed(const std::string& entity_id) const;
    float getSkillBonus(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::SalvageProcessingState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SALVAGE_PROCESSING_SYSTEM_H
