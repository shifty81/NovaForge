#ifndef NOVAFORGE_SYSTEMS_SCAN_PROBE_DEPLOYMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SCAN_PROBE_DEPLOYMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Scan probe deployment for discovering anomalies and signatures
 *
 * Players launch scan probes into a system to resolve hidden signatures
 * (anomalies, wormholes, data/relic sites, gas clouds).  Each probe
 * has a scan radius and strength; overlapping coverage increases
 * resolution speed.  Probes auto-recall after their lifetime expires.
 * Essential for the exploration loop in the vertical slice.
 */
class ScanProbeDeploymentSystem : public ecs::SingleComponentSystem<components::ScanProbeDeploymentState> {
public:
    explicit ScanProbeDeploymentSystem(ecs::World* world);
    ~ScanProbeDeploymentSystem() override = default;

    std::string getName() const override { return "ScanProbeDeploymentSystem"; }

public:
    // Initialization
    bool initialize(const std::string& entity_id);

    // Probe management
    bool launchProbe(const std::string& entity_id, const std::string& probe_id,
                     float x, float y, float z, float scan_radius, float scan_strength);
    bool recallProbe(const std::string& entity_id, const std::string& probe_id);
    bool recallAllProbes(const std::string& entity_id);

    // Signature management
    bool addSignature(const std::string& entity_id, const std::string& sig_id,
                      const std::string& sig_type, float x, float y, float z);

    // Queries
    int getActiveProbeCount(const std::string& entity_id) const;
    int getSignatureCount(const std::string& entity_id) const;
    int getResolvedSignatureCount(const std::string& entity_id) const;
    float getSignatureScanProgress(const std::string& entity_id,
                                    const std::string& sig_id) const;
    bool isSignatureResolved(const std::string& entity_id,
                              const std::string& sig_id) const;
    int getTotalProbesLaunched(const std::string& entity_id) const;
    int getTotalSignaturesResolved(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ScanProbeDeploymentState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SCAN_PROBE_DEPLOYMENT_SYSTEM_H
