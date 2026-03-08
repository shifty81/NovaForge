#include "systems/network_quality_monitor_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

NetworkQualityMonitorSystem::NetworkQualityMonitorSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NetworkQualityMonitorSystem::updateComponent(ecs::Entity& /*entity*/,
    components::NetworkQualityState& state, float delta_time) {
    if (!state.active) return;
    state.elapsed += delta_time;

    // Evaluate degraded status
    bool bad_latency = state.latency_ms > m_latency_threshold;
    bool bad_jitter  = state.jitter_ms  > m_jitter_threshold;
    bool bad_loss    = state.packet_loss_pct > m_loss_threshold;
    state.degraded = bad_latency || bad_jitter || bad_loss;

    // Adapt snapshot rate: degrade → half rate, good → full 20 Hz
    if (state.degraded) {
        state.snapshot_rate_hz = std::max(5.0f, state.snapshot_rate_hz * 0.9f);
    } else {
        state.snapshot_rate_hz = std::min(20.0f, state.snapshot_rate_hz * 1.1f);
    }
}

bool NetworkQualityMonitorSystem::initialize(const std::string& entity_id,
    const std::string& connection_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (connection_id.empty()) return false;
    auto comp = std::make_unique<components::NetworkQualityState>();
    comp->connection_id = connection_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool NetworkQualityMonitorSystem::recordLatencySample(const std::string& entity_id,
    float latency_ms) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (latency_ms < 0.0f) return false;

    // Exponential moving average for latency
    float alpha = 0.2f;
    float prev = state->latency_ms;
    state->latency_ms = state->samples_received == 0
        ? latency_ms
        : (1.0f - alpha) * prev + alpha * latency_ms;

    // Jitter = smoothed |delta|
    float delta = std::fabs(latency_ms - prev);
    state->jitter_ms = state->samples_received == 0
        ? 0.0f
        : (1.0f - alpha) * state->jitter_ms + alpha * delta;

    state->min_latency_ms = std::min(state->min_latency_ms, latency_ms);
    state->max_latency_ms = std::max(state->max_latency_ms, latency_ms);
    state->samples_received++;
    return true;
}

bool NetworkQualityMonitorSystem::recordPacketLoss(const std::string& entity_id,
    int lost, int total) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (lost < 0 || total <= 0) return false;
    state->packets_lost += lost;
    state->packets_total += total;
    state->packet_loss_pct = 100.0f * static_cast<float>(state->packets_lost)
                                    / static_cast<float>(state->packets_total);
    return true;
}

bool NetworkQualityMonitorSystem::resetStats(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->latency_ms = 0.0f;
    state->jitter_ms = 0.0f;
    state->packet_loss_pct = 0.0f;
    state->snapshot_rate_hz = 20.0f;
    state->samples_received = 0;
    state->packets_lost = 0;
    state->packets_total = 0;
    state->min_latency_ms = 999.0f;
    state->max_latency_ms = 0.0f;
    state->degraded = false;
    return true;
}

float NetworkQualityMonitorSystem::getLatency(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->latency_ms : 0.0f;
}

float NetworkQualityMonitorSystem::getJitter(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->jitter_ms : 0.0f;
}

float NetworkQualityMonitorSystem::getPacketLossPct(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->packet_loss_pct : 0.0f;
}

float NetworkQualityMonitorSystem::getSnapshotRate(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->snapshot_rate_hz : 20.0f;
}

int NetworkQualityMonitorSystem::getSamplesReceived(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->samples_received : 0;
}

bool NetworkQualityMonitorSystem::isDegraded(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->degraded : false;
}

float NetworkQualityMonitorSystem::getMinLatency(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->min_latency_ms : 0.0f;
}

float NetworkQualityMonitorSystem::getMaxLatency(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->max_latency_ms : 0.0f;
}

std::string NetworkQualityMonitorSystem::getConnectionId(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->connection_id : "";
}

} // namespace systems
} // namespace atlas
