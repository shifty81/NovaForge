#include "systems/fleet_after_action_report_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FleetAfterActionReportSystem::FleetAfterActionReportSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void FleetAfterActionReportSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::FleetAfterActionReport& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool FleetAfterActionReportSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    entity->addComponent(
        std::make_unique<components::FleetAfterActionReport>());
    return true;
}

bool FleetAfterActionReportSystem::startReport(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    // Reset for a new engagement
    comp->state         = components::FleetAfterActionReport::State::Recording;
    comp->members.clear();
    comp->total_kills   = 0;
    comp->total_losses  = 0;
    comp->total_damage  = 0.0f;
    comp->total_loot    = 0.0f;
    comp->duration      = 0.0f;
    return true;
}

bool FleetAfterActionReportSystem::finalizeReport(const std::string& entity_id,
                                                   float duration) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::FleetAfterActionReport::State::Recording)
        return false;
    comp->duration = duration >= 0.0f ? duration : 0.0f;
    comp->state = components::FleetAfterActionReport::State::Finalized;
    ++comp->total_reports;
    return true;
}

// ---------------------------------------------------------------------------
// Member registration
// ---------------------------------------------------------------------------

bool FleetAfterActionReportSystem::addMember(const std::string& entity_id,
                                              const std::string& pilot_id) {
    if (pilot_id.empty()) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state == components::FleetAfterActionReport::State::Finalized)
        return false;
    if (static_cast<int>(comp->members.size()) >= comp->max_members)
        return false;
    for (const auto& m : comp->members) {
        if (m.pilot_id == pilot_id) return false; // duplicate
    }
    components::FleetAfterActionReport::MemberStats s;
    s.pilot_id = pilot_id;
    comp->members.push_back(s);
    return true;
}

// ---------------------------------------------------------------------------
// Event recording helpers
// ---------------------------------------------------------------------------

static components::FleetAfterActionReport::MemberStats*
findMember(components::FleetAfterActionReport& comp,
           const std::string& pilot_id) {
    for (auto& m : comp.members) {
        if (m.pilot_id == pilot_id) return &m;
    }
    return nullptr;
}

bool FleetAfterActionReportSystem::recordKill(const std::string& entity_id,
                                               const std::string& pilot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::FleetAfterActionReport::State::Recording)
        return false;
    auto* m = findMember(*comp, pilot_id);
    if (!m) return false;
    ++m->kills;
    ++comp->total_kills;
    return true;
}

bool FleetAfterActionReportSystem::recordLoss(const std::string& entity_id,
                                               const std::string& pilot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::FleetAfterActionReport::State::Recording)
        return false;
    auto* m = findMember(*comp, pilot_id);
    if (!m) return false;
    ++m->losses;
    ++comp->total_losses;
    return true;
}

bool FleetAfterActionReportSystem::recordDamageDealt(
        const std::string& entity_id,
        const std::string& pilot_id,
        float amount) {
    if (amount < 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::FleetAfterActionReport::State::Recording)
        return false;
    auto* m = findMember(*comp, pilot_id);
    if (!m) return false;
    m->damage_dealt  += amount;
    comp->total_damage += amount;
    return true;
}

bool FleetAfterActionReportSystem::recordDamageReceived(
        const std::string& entity_id,
        const std::string& pilot_id,
        float amount) {
    if (amount < 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::FleetAfterActionReport::State::Recording)
        return false;
    auto* m = findMember(*comp, pilot_id);
    if (!m) return false;
    m->damage_received += amount;
    return true;
}

bool FleetAfterActionReportSystem::recordLootShared(
        const std::string& entity_id,
        const std::string& pilot_id,
        float isk_value) {
    if (isk_value < 0.0f) return false;
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->state != components::FleetAfterActionReport::State::Recording)
        return false;
    auto* m = findMember(*comp, pilot_id);
    if (!m) return false;
    m->loot_shared  += isk_value;
    comp->total_loot += isk_value;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

components::FleetAfterActionReport::State
FleetAfterActionReportSystem::getState(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->state
                : components::FleetAfterActionReport::State::Idle;
}

int FleetAfterActionReportSystem::getMemberCount(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->members.size()) : 0;
}

int FleetAfterActionReportSystem::getTotalKills(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_kills : 0;
}

int FleetAfterActionReportSystem::getTotalLosses(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_losses : 0;
}

float FleetAfterActionReportSystem::getTotalDamage(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_damage : 0.0f;
}

float FleetAfterActionReportSystem::getTotalLoot(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_loot : 0.0f;
}

int FleetAfterActionReportSystem::getTotalReports(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_reports : 0;
}

std::string FleetAfterActionReportSystem::getMVP(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp || comp->members.empty()) return {};
    const components::FleetAfterActionReport::MemberStats* best =
        &comp->members[0];
    for (const auto& m : comp->members) {
        if (m.damage_dealt > best->damage_dealt) best = &m;
    }
    return best->pilot_id;
}

float FleetAfterActionReportSystem::getFleetEfficiency(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    int total = comp->total_kills + comp->total_losses;
    if (total == 0) return 0.0f;
    return static_cast<float>(comp->total_kills) / static_cast<float>(total);
}

} // namespace systems
} // namespace atlas
