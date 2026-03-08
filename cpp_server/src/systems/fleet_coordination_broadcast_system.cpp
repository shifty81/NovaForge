#include "systems/fleet_coordination_broadcast_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/ship_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FleetCoordinationBroadcastSystem::FleetCoordinationBroadcastSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void FleetCoordinationBroadcastSystem::updateComponent(ecs::Entity& entity,
    components::FleetCoordinationState& fcs, float delta_time) {
    if (!fcs.active) return;

    // Expire old signals
    fcs.active_signals.erase(
        std::remove_if(fcs.active_signals.begin(), fcs.active_signals.end(),
            [&](components::FleetCoordinationState::Signal& sig) {
                sig.timestamp += delta_time;
                return sig.timestamp >= sig.duration;
            }),
        fcs.active_signals.end());

    // Decay signal strength over time
    fcs.signal_strength = std::max(0.0f, std::min(1.0f, fcs.signal_strength));
    fcs.elapsed += delta_time;
}

bool FleetCoordinationBroadcastSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::FleetCoordinationState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool FleetCoordinationBroadcastSystem::broadcastSignal(const std::string& entity_id,
    const std::string& signal_type, const std::string& issuer_id) {
    auto* fcs = getComponentFor(entity_id);
    if (!fcs) return false;
    if (signal_type != "rally" && signal_type != "retreat" && signal_type != "regroup"
        && signal_type != "hold" && signal_type != "advance") return false;
    if (issuer_id.empty()) return false;
    if (static_cast<int>(fcs->active_signals.size()) >= fcs->max_signals) return false;

    components::FleetCoordinationState::Signal sig;
    sig.signal_type = signal_type;
    sig.issuer_id = issuer_id;
    sig.timestamp = 0.0f;
    sig.duration = 30.0f;
    fcs->active_signals.push_back(sig);
    fcs->total_broadcasts++;
    return true;
}

bool FleetCoordinationBroadcastSystem::acknowledgeSignal(const std::string& entity_id,
    const std::string& signal_type) {
    auto* fcs = getComponentFor(entity_id);
    if (!fcs) return false;
    for (auto& sig : fcs->active_signals) {
        if (sig.signal_type == signal_type) {
            fcs->total_acknowledged++;
            return true;
        }
    }
    return false;
}

bool FleetCoordinationBroadcastSystem::clearSignals(const std::string& entity_id) {
    auto* fcs = getComponentFor(entity_id);
    if (!fcs) return false;
    fcs->active_signals.clear();
    return true;
}

bool FleetCoordinationBroadcastSystem::setBroadcastRange(const std::string& entity_id, float range) {
    auto* fcs = getComponentFor(entity_id);
    if (!fcs) return false;
    fcs->broadcast_range = std::max(0.0f, std::min(500.0f, range));
    return true;
}

bool FleetCoordinationBroadcastSystem::setSignalStrength(const std::string& entity_id, float strength) {
    auto* fcs = getComponentFor(entity_id);
    if (!fcs) return false;
    fcs->signal_strength = std::max(0.0f, std::min(1.0f, strength));
    return true;
}

float FleetCoordinationBroadcastSystem::getBroadcastRange(const std::string& entity_id) const {
    auto* fcs = getComponentFor(entity_id);
    return fcs ? fcs->broadcast_range : 0.0f;
}

float FleetCoordinationBroadcastSystem::getSignalStrength(const std::string& entity_id) const {
    auto* fcs = getComponentFor(entity_id);
    return fcs ? fcs->signal_strength : 0.0f;
}

int FleetCoordinationBroadcastSystem::getActiveSignalCount(const std::string& entity_id) const {
    auto* fcs = getComponentFor(entity_id);
    return fcs ? static_cast<int>(fcs->active_signals.size()) : 0;
}

int FleetCoordinationBroadcastSystem::getTotalBroadcasts(const std::string& entity_id) const {
    auto* fcs = getComponentFor(entity_id);
    return fcs ? fcs->total_broadcasts : 0;
}

int FleetCoordinationBroadcastSystem::getTotalAcknowledged(const std::string& entity_id) const {
    auto* fcs = getComponentFor(entity_id);
    return fcs ? fcs->total_acknowledged : 0;
}

bool FleetCoordinationBroadcastSystem::hasActiveSignal(const std::string& entity_id,
    const std::string& signal_type) const {
    auto* fcs = getComponentFor(entity_id);
    if (!fcs) return false;
    for (const auto& sig : fcs->active_signals) {
        if (sig.signal_type == signal_type) return true;
    }
    return false;
}

} // namespace systems
} // namespace atlas
