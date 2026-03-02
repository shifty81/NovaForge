#ifndef NOVAFORGE_SYSTEMS_SCAN_PROBE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SCAN_PROBE_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Scan probe deployment and management for exploration
 *
 * Deploy scan probes in space to locate hidden sites, wormholes, and anomalies.
 * Probes have scan time, radius, and lifetime. Results accumulate as signal
 * strength increases with successive scans.
 */
class ScanProbeSystem : public ecs::System {
public:
    explicit ScanProbeSystem(ecs::World* world);
    ~ScanProbeSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ScanProbeSystem"; }

    bool initializeProbes(const std::string& entity_id, const std::string& owner_id);
    bool launchProbe(const std::string& entity_id, const std::string& probe_id,
                     int probe_type, float x, float y, float z);
    bool recallProbe(const std::string& entity_id, const std::string& probe_id);
    bool startScan(const std::string& entity_id, const std::string& probe_id);
    int getProbeCount(const std::string& entity_id) const;
    int getActiveProbeCount(const std::string& entity_id) const;
    float getScanProgress(const std::string& entity_id, const std::string& probe_id) const;
    bool addResult(const std::string& entity_id, const std::string& result_id,
                   const std::string& sig_type, float strength);
    int getResultCount(const std::string& entity_id) const;
    int getTotalScansCompleted(const std::string& entity_id) const;
    int getTotalSitesFound(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SCAN_PROBE_SYSTEM_H
