#ifndef NOVAFORGE_SYSTEMS_NETWORK_QUALITY_MONITOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_NETWORK_QUALITY_MONITOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/navigation_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Tracks per-connection network quality and adapts snapshot rate
 *
 * Receives latency samples, computes smoothed RTT and jitter, detects
 * packet loss, and adjusts the snapshot send rate for each player
 * connection.  When quality drops below configurable thresholds the
 * connection is marked "degraded" and snapshot rate is lowered to
 * conserve bandwidth.
 *
 * Thresholds (configurable):
 *   latency_threshold_ms = 200   → degrade above this
 *   jitter_threshold_ms  = 50    → degrade above this
 *   packet_loss_threshold = 5.0  → degrade above this (%)
 */
class NetworkQualityMonitorSystem : public ecs::SingleComponentSystem<components::NetworkQualityState> {
public:
    explicit NetworkQualityMonitorSystem(ecs::World* world);
    ~NetworkQualityMonitorSystem() override = default;

    std::string getName() const override { return "NetworkQualityMonitorSystem"; }

    bool initialize(const std::string& entity_id, const std::string& connection_id);
    bool recordLatencySample(const std::string& entity_id, float latency_ms);
    bool recordPacketLoss(const std::string& entity_id, int lost, int total);
    bool resetStats(const std::string& entity_id);

    float getLatency(const std::string& entity_id) const;
    float getJitter(const std::string& entity_id) const;
    float getPacketLossPct(const std::string& entity_id) const;
    float getSnapshotRate(const std::string& entity_id) const;
    int   getSamplesReceived(const std::string& entity_id) const;
    bool  isDegraded(const std::string& entity_id) const;
    float getMinLatency(const std::string& entity_id) const;
    float getMaxLatency(const std::string& entity_id) const;
    std::string getConnectionId(const std::string& entity_id) const;

    void setLatencyThreshold(float ms) { m_latency_threshold = ms; }
    void setJitterThreshold(float ms) { m_jitter_threshold = ms; }
    void setPacketLossThreshold(float pct) { m_loss_threshold = pct; }

protected:
    void updateComponent(ecs::Entity& entity, components::NetworkQualityState& state, float delta_time) override;

private:
    float m_latency_threshold = 200.0f;
    float m_jitter_threshold  = 50.0f;
    float m_loss_threshold    = 5.0f;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_NETWORK_QUALITY_MONITOR_SYSTEM_H
