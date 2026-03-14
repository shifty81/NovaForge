#ifndef NOVAFORGE_SYSTEMS_MOON_MINING_SCHEDULER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_MOON_MINING_SCHEDULER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/economy_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Moon mining extraction scheduling system
 *
 * Manages moon drill extraction cycles for Refinery structures.  An
 * extraction is started with startExtraction which schedules the drill
 * duration.  Per-tick countdown advances time_remaining; when it reaches
 * zero the extraction status transitions to Ready.  fractureExtraction
 * detonates the chunk (status → Fractured) and accumulates yield.
 * cancelExtraction aborts an in-progress extraction.  max_extractions
 * (default 10) caps the history.
 */
class MoonMiningSchedulerSystem
    : public ecs::SingleComponentSystem<components::MoonMiningSchedulerState> {
public:
    explicit MoonMiningSchedulerSystem(ecs::World* world);
    ~MoonMiningSchedulerSystem() override = default;

    std::string getName() const override { return "MoonMiningSchedulerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Extraction management ---
    bool startExtraction(const std::string& entity_id,
                         const std::string& extraction_id,
                         const std::string& moon_id,
                         const std::string& ore_type,
                         float duration,
                         float estimated_yield);
    bool cancelExtraction(const std::string& entity_id,
                          const std::string& extraction_id);
    bool fractureExtraction(const std::string& entity_id,
                            const std::string& extraction_id);
    bool removeExtraction(const std::string& entity_id,
                          const std::string& extraction_id);
    bool clearExtractions(const std::string& entity_id);

    // --- Configuration ---
    bool setStructureId(const std::string& entity_id,
                        const std::string& structure_id);
    bool setMaxExtractions(const std::string& entity_id, int max_extractions);

    // --- Queries ---
    int   getExtractionCount(const std::string& entity_id) const;
    int   getActiveExtractionCount(const std::string& entity_id) const;
    int   getTotalExtractionsStarted(const std::string& entity_id) const;
    int   getTotalFractured(const std::string& entity_id) const;
    float getTotalYield(const std::string& entity_id) const;
    float getTimeRemaining(const std::string& entity_id,
                           const std::string& extraction_id) const;
    bool  hasExtraction(const std::string& entity_id,
                        const std::string& extraction_id) const;
    std::string getStructureId(const std::string& entity_id) const;
    int   getCountByStatus(const std::string& entity_id,
                           components::MoonMiningSchedulerState::ExtractionStatus status) const;
    std::string getExtractionStatus(const std::string& entity_id,
                                    const std::string& extraction_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::MoonMiningSchedulerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_MOON_MINING_SCHEDULER_SYSTEM_H
