#ifndef NOVAFORGE_SYSTEMS_SURVEY_SCANNER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SURVEY_SCANNER_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Asteroid-belt survey scanner system
 *
 * Scans asteroid belts to discover ore types, quantities, and estimated ISK
 * values.  Starting a scan sets the state to Scanning and begins a timer
 * that counts up to scan_duration; on completion the state moves to
 * Complete.  Each scan produces a SurveyResult entry.  Results are capped at
 * max_results (default 20); inserts are rejected when at capacity.  A scan
 * may only be started when the scanner is Idle.  Lifetime counters track
 * total scans completed and total estimated value scanned.
 */
class SurveyScannerSystem
    : public ecs::SingleComponentSystem<components::SurveyScannerState> {
public:
    explicit SurveyScannerSystem(ecs::World* world);
    ~SurveyScannerSystem() override = default;

    std::string getName() const override { return "SurveyScannerSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Scanning ---
    bool startScan(const std::string& entity_id,
                   const std::string& target_belt_id);
    bool cancelScan(const std::string& entity_id);
    bool addResult(const std::string& entity_id,
                   const std::string& result_id,
                   const std::string& asteroid_id,
                   const std::string& ore_type,
                   float quantity,
                   float estimated_value,
                   float distance);
    bool removeResult(const std::string& entity_id,
                      const std::string& result_id);
    bool clearResults(const std::string& entity_id);

    // --- Configuration ---
    bool setScanDuration(const std::string& entity_id, float duration);
    bool setScanRange(const std::string& entity_id, float range);
    bool setMaxResults(const std::string& entity_id, int max_results);
    bool setScanDeviation(const std::string& entity_id, float deviation);

    // --- Queries ---
    int   getResultCount(const std::string& entity_id) const;
    bool  hasResult(const std::string& entity_id,
                    const std::string& result_id) const;
    float getScanProgress(const std::string& entity_id) const;
    bool  isScanning(const std::string& entity_id) const;
    bool  isScanComplete(const std::string& entity_id) const;
    int   getTotalScansCompleted(const std::string& entity_id) const;
    float getTotalValueScanned(const std::string& entity_id) const;
    std::string getTargetBeltId(const std::string& entity_id) const;
    float getScanDuration(const std::string& entity_id) const;
    float getScanRange(const std::string& entity_id) const;
    float getResultQuantity(const std::string& entity_id,
                            const std::string& result_id) const;
    std::string getResultOreType(const std::string& entity_id,
                                 const std::string& result_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SurveyScannerState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SURVEY_SCANNER_SYSTEM_H
