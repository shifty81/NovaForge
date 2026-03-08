#include "systems/scan_probe_deployment_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ScanProbeDeploymentSystem::ScanProbeDeploymentSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ScanProbeDeploymentSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ScanProbeDeploymentState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Age probes and auto-recall expired ones
    for (auto& probe : comp.probes) {
        if (probe.recalled) continue;
        probe.age += delta_time;
        if (probe.age >= probe.lifetime) {
            probe.recalled = true;
        }
    }

    // Advance scan progress on signatures within active probe range
    for (auto& sig : comp.signatures) {
        if (sig.resolved) continue;

        float total_strength = 0.0f;
        for (const auto& probe : comp.probes) {
            if (probe.recalled) continue;

            float dx = sig.x - probe.x;
            float dy = sig.y - probe.y;
            float dz = sig.z - probe.z;
            float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

            if (dist <= probe.scan_radius) {
                // Closer probes contribute more
                float coverage = 1.0f - (dist / probe.scan_radius);
                total_strength += probe.scan_strength * coverage;
            }
        }

        if (total_strength > 0.0f) {
            sig.scan_progress += total_strength * delta_time * 0.1f; // 10% per strength per second
            if (sig.scan_progress >= 1.0f) {
                sig.scan_progress = 1.0f;
                sig.resolved = true;
                comp.total_signatures_resolved++;
            }
        }
    }
}

bool ScanProbeDeploymentSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::ScanProbeDeploymentState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool ScanProbeDeploymentSystem::launchProbe(const std::string& entity_id,
    const std::string& probe_id, float x, float y, float z,
    float scan_radius, float scan_strength) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (probe_id.empty()) return false;
    if (scan_radius <= 0.0f || scan_strength <= 0.0f) return false;

    // Check for duplicate probe ID
    for (const auto& p : comp->probes) {
        if (p.probe_id == probe_id && !p.recalled) return false;
    }

    // Count active probes
    int active = 0;
    for (const auto& p : comp->probes) {
        if (!p.recalled) active++;
    }
    if (active >= comp->max_probes) return false;

    components::ScanProbeDeploymentState::ScanProbe probe;
    probe.probe_id = probe_id;
    probe.x = x;
    probe.y = y;
    probe.z = z;
    probe.scan_radius = scan_radius;
    probe.scan_strength = scan_strength;
    comp->probes.push_back(probe);
    comp->total_probes_launched++;
    return true;
}

bool ScanProbeDeploymentSystem::recallProbe(const std::string& entity_id,
    const std::string& probe_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& p : comp->probes) {
        if (p.probe_id == probe_id && !p.recalled) {
            p.recalled = true;
            return true;
        }
    }
    return false;
}

bool ScanProbeDeploymentSystem::recallAllProbes(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    bool any = false;
    for (auto& p : comp->probes) {
        if (!p.recalled) {
            p.recalled = true;
            any = true;
        }
    }
    return any;
}

bool ScanProbeDeploymentSystem::addSignature(const std::string& entity_id,
    const std::string& sig_id, const std::string& sig_type,
    float x, float y, float z) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (sig_id.empty() || sig_type.empty()) return false;

    // Check for duplicate
    for (const auto& s : comp->signatures) {
        if (s.sig_id == sig_id) return false;
    }

    if (static_cast<int>(comp->signatures.size()) >= comp->max_signatures) return false;

    components::ScanProbeDeploymentState::Signature sig;
    sig.sig_id = sig_id;
    sig.sig_type = sig_type;
    sig.x = x;
    sig.y = y;
    sig.z = z;
    comp->signatures.push_back(sig);
    return true;
}

int ScanProbeDeploymentSystem::getActiveProbeCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& p : comp->probes) {
        if (!p.recalled) count++;
    }
    return count;
}

int ScanProbeDeploymentSystem::getSignatureCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->signatures.size()) : 0;
}

int ScanProbeDeploymentSystem::getResolvedSignatureCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& s : comp->signatures) {
        if (s.resolved) count++;
    }
    return count;
}

float ScanProbeDeploymentSystem::getSignatureScanProgress(const std::string& entity_id,
    const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& s : comp->signatures) {
        if (s.sig_id == sig_id) return s.scan_progress;
    }
    return 0.0f;
}

bool ScanProbeDeploymentSystem::isSignatureResolved(const std::string& entity_id,
    const std::string& sig_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    for (const auto& s : comp->signatures) {
        if (s.sig_id == sig_id) return s.resolved;
    }
    return false;
}

int ScanProbeDeploymentSystem::getTotalProbesLaunched(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_probes_launched : 0;
}

int ScanProbeDeploymentSystem::getTotalSignaturesResolved(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_signatures_resolved : 0;
}

} // namespace systems
} // namespace atlas
