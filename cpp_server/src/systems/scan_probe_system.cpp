#include "systems/scan_probe_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::ScanProbe::Probe* findProbe(
    components::ScanProbe* sp, const std::string& probe_id) {
    for (auto& p : sp->probes) {
        if (p.probe_id == probe_id) return &p;
    }
    return nullptr;
}

const components::ScanProbe::Probe* findProbeConst(
    const components::ScanProbe* sp, const std::string& probe_id) {
    for (const auto& p : sp->probes) {
        if (p.probe_id == probe_id) return &p;
    }
    return nullptr;
}
} // anonymous namespace

ScanProbeSystem::ScanProbeSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void ScanProbeSystem::updateComponent(ecs::Entity& /*entity*/, components::ScanProbe& sp, float delta_time) {
    if (!sp.active) return;

    for (auto& probe : sp.probes) {
        // Decrement lifetime
        probe.lifetime -= delta_time;
        if (probe.lifetime <= 0.0f) {
            probe.lifetime = 0.0f;
            probe.state = components::ScanProbe::ProbeState::Expired;
            continue;
        }

        // Advance scanning
        if (probe.state == components::ScanProbe::ProbeState::Scanning) {
            probe.scan_progress += (delta_time / probe.scan_time) * probe.scan_strength;
            if (probe.scan_progress >= 1.0f) {
                probe.scan_progress = 1.0f;
                probe.state = components::ScanProbe::ProbeState::Complete;
                sp.total_scans_completed++;
            }
        }
    }

    // Remove expired probes
    sp.probes.erase(
        std::remove_if(sp.probes.begin(), sp.probes.end(),
            [](const components::ScanProbe::Probe& p) {
                return p.state == components::ScanProbe::ProbeState::Expired;
            }),
        sp.probes.end());
}

bool ScanProbeSystem::initializeProbes(const std::string& entity_id,
    const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::ScanProbe>();
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool ScanProbeSystem::launchProbe(const std::string& entity_id,
    const std::string& probe_id, int probe_type, float x, float y, float z) {
    auto* sp = getComponentFor(entity_id);
    if (!sp) return false;
    if (static_cast<int>(sp->probes.size()) >= sp->max_probes) return false;
    if (findProbe(sp, probe_id)) return false; // duplicate

    components::ScanProbe::Probe probe;
    probe.probe_id = probe_id;
    probe.type = static_cast<components::ScanProbe::ProbeType>(probe_type);
    probe.state = components::ScanProbe::ProbeState::Idle;
    probe.x = x;
    probe.y = y;
    probe.z = z;
    sp->probes.push_back(probe);
    return true;
}

bool ScanProbeSystem::recallProbe(const std::string& entity_id,
    const std::string& probe_id) {
    auto* sp = getComponentFor(entity_id);
    if (!sp) return false;

    auto it = std::remove_if(sp->probes.begin(), sp->probes.end(),
        [&](const components::ScanProbe::Probe& p) {
            return p.probe_id == probe_id;
        });
    if (it == sp->probes.end()) return false;
    sp->probes.erase(it, sp->probes.end());
    return true;
}

bool ScanProbeSystem::startScan(const std::string& entity_id,
    const std::string& probe_id) {
    auto* sp = getComponentFor(entity_id);
    if (!sp) return false;

    auto* probe = findProbe(sp, probe_id);
    if (!probe) return false;
    if (probe->state != components::ScanProbe::ProbeState::Idle &&
        probe->state != components::ScanProbe::ProbeState::Complete) return false;

    probe->state = components::ScanProbe::ProbeState::Scanning;
    probe->scan_progress = 0.0f;
    return true;
}

int ScanProbeSystem::getProbeCount(const std::string& entity_id) const {
    const auto* sp = getComponentFor(entity_id);
    if (!sp) return 0;
    return static_cast<int>(sp->probes.size());
}

int ScanProbeSystem::getActiveProbeCount(const std::string& entity_id) const {
    const auto* sp = getComponentFor(entity_id);
    if (!sp) return 0;
    int count = 0;
    for (const auto& p : sp->probes) {
        if (p.state == components::ScanProbe::ProbeState::Scanning) count++;
    }
    return count;
}

float ScanProbeSystem::getScanProgress(const std::string& entity_id,
    const std::string& probe_id) const {
    const auto* sp = getComponentFor(entity_id);
    if (!sp) return 0.0f;
    auto* probe = findProbeConst(sp, probe_id);
    return probe ? probe->scan_progress : 0.0f;
}

bool ScanProbeSystem::addResult(const std::string& entity_id,
    const std::string& result_id, const std::string& sig_type, float strength) {
    auto* sp = getComponentFor(entity_id);
    if (!sp) return false;

    // Check for duplicate
    for (const auto& r : sp->results) {
        if (r.result_id == result_id) return false;
    }

    components::ScanProbe::ScanResult result;
    result.result_id = result_id;
    result.signature_type = sig_type;
    result.signal_strength = strength;
    sp->results.push_back(result);
    sp->total_sites_found++;
    return true;
}

int ScanProbeSystem::getResultCount(const std::string& entity_id) const {
    const auto* sp = getComponentFor(entity_id);
    if (!sp) return 0;
    return static_cast<int>(sp->results.size());
}

int ScanProbeSystem::getTotalScansCompleted(const std::string& entity_id) const {
    const auto* sp = getComponentFor(entity_id);
    if (!sp) return 0;
    return sp->total_scans_completed;
}

int ScanProbeSystem::getTotalSitesFound(const std::string& entity_id) const {
    const auto* sp = getComponentFor(entity_id);
    if (!sp) return 0;
    return sp->total_sites_found;
}

} // namespace systems
} // namespace atlas
