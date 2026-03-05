#include "systems/commander_disagreement_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

CommanderDisagreementSystem::CommanderDisagreementSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void CommanderDisagreementSystem::updateComponent(ecs::Entity& /*entity*/, components::CommanderDisagreement& comp, float delta_time) {
    for (auto& d : comp.disagreements) {
        if (d.resolved) continue;

        d.timer += delta_time;

        // Escalate severity over time
        if (d.timer >= d.escalation_threshold) {
            if (d.severity == components::CommanderDisagreement::Severity::Minor) {
                d.severity = components::CommanderDisagreement::Severity::Moderate;
                d.morale_impact -= 5.0f;
            } else if (d.severity == components::CommanderDisagreement::Severity::Moderate) {
                d.severity = components::CommanderDisagreement::Severity::Serious;
                d.morale_impact -= 10.0f;
            } else if (d.severity == components::CommanderDisagreement::Severity::Serious) {
                d.severity = components::CommanderDisagreement::Severity::Critical;
                d.resolution = components::CommanderDisagreement::Resolution::Escalated;
                d.morale_impact -= 20.0f;
                d.resolved = true;
                comp.total_resolved++;
            }
            d.timer = 0.0f;
        }

        // Accumulate tension from unresolved disagreements
        float severity_weight = 0.0f;
        switch (d.severity) {
            case components::CommanderDisagreement::Severity::Minor: severity_weight = 0.5f; break;
            case components::CommanderDisagreement::Severity::Moderate: severity_weight = 1.0f; break;
            case components::CommanderDisagreement::Severity::Serious: severity_weight = 2.0f; break;
            case components::CommanderDisagreement::Severity::Critical: severity_weight = 5.0f; break;
        }
        comp.fleet_tension += severity_weight * delta_time;
    }

    comp.fleet_tension = std::clamp(comp.fleet_tension, 0.0f, 100.0f);
}

static components::CommanderDisagreement::Topic parseTopicString(const std::string& topic) {
    if (topic == "Target") return components::CommanderDisagreement::Topic::Target;
    if (topic == "Formation") return components::CommanderDisagreement::Topic::Formation;
    if (topic == "Retreat") return components::CommanderDisagreement::Topic::Retreat;
    if (topic == "LootSplit") return components::CommanderDisagreement::Topic::LootSplit;
    return components::CommanderDisagreement::Topic::Strategy;
}

static components::CommanderDisagreement::Resolution parseResolutionString(const std::string& res) {
    if (res == "Vote") return components::CommanderDisagreement::Resolution::Vote;
    if (res == "AuthorityOverride") return components::CommanderDisagreement::Resolution::AuthorityOverride;
    if (res == "Compromise") return components::CommanderDisagreement::Resolution::Compromise;
    if (res == "Escalated") return components::CommanderDisagreement::Resolution::Escalated;
    return components::CommanderDisagreement::Resolution::None;
}

bool CommanderDisagreementSystem::raiseDisagreement(const std::string& fleet_id,
                                                     const std::string& commander_a,
                                                     const std::string& commander_b,
                                                     const std::string& topic) {
    auto* cd = getComponentFor(fleet_id);
    if (!cd) return false;

    components::CommanderDisagreement::Disagreement d;
    d.commander_a_id = commander_a;
    d.commander_b_id = commander_b;
    d.topic = parseTopicString(topic);
    d.severity = components::CommanderDisagreement::Severity::Minor;
    d.resolution = components::CommanderDisagreement::Resolution::None;
    d.timer = 0.0f;
    d.resolved = false;
    d.morale_impact = -2.0f;  // initial minor impact

    cd->disagreements.push_back(d);
    cd->total_disagreements++;
    cd->fleet_tension += 5.0f;
    cd->fleet_tension = std::clamp(cd->fleet_tension, 0.0f, 100.0f);
    return true;
}

bool CommanderDisagreementSystem::resolveDisagreement(const std::string& fleet_id, int index,
                                                       const std::string& resolution) {
    auto* cd = getComponentFor(fleet_id);
    if (!cd) return false;

    if (index < 0 || index >= static_cast<int>(cd->disagreements.size())) return false;
    auto& d = cd->disagreements[index];
    if (d.resolved) return false;

    d.resolution = parseResolutionString(resolution);
    d.resolved = true;
    cd->total_resolved++;

    // Resolution reduces tension
    switch (d.resolution) {
        case components::CommanderDisagreement::Resolution::Compromise:
            cd->fleet_tension = std::max(0.0f, cd->fleet_tension - 10.0f);
            d.morale_impact += 3.0f;  // compromise partially recovers morale
            break;
        case components::CommanderDisagreement::Resolution::Vote:
            cd->fleet_tension = std::max(0.0f, cd->fleet_tension - 7.0f);
            d.morale_impact += 1.0f;
            break;
        case components::CommanderDisagreement::Resolution::AuthorityOverride:
            cd->fleet_tension = std::max(0.0f, cd->fleet_tension - 3.0f);
            d.morale_impact -= 3.0f;  // authority override hurts morale more
            break;
        default:
            break;
    }

    return true;
}

bool CommanderDisagreementSystem::dismissDisagreement(const std::string& fleet_id, int index) {
    auto* cd = getComponentFor(fleet_id);
    if (!cd) return false;

    if (index < 0 || index >= static_cast<int>(cd->disagreements.size())) return false;
    cd->disagreements.erase(cd->disagreements.begin() + index);
    return true;
}

int CommanderDisagreementSystem::getActiveCount(const std::string& fleet_id) const {
    const auto* cd = getComponentFor(fleet_id);
    if (!cd) return 0;

    int count = 0;
    for (const auto& d : cd->disagreements) {
        if (!d.resolved) count++;
    }
    return count;
}

float CommanderDisagreementSystem::getFleetTension(const std::string& fleet_id) const {
    const auto* cd = getComponentFor(fleet_id);
    return cd ? cd->fleet_tension : 0.0f;
}

int CommanderDisagreementSystem::getTotalDisagreements(const std::string& fleet_id) const {
    const auto* cd = getComponentFor(fleet_id);
    return cd ? cd->total_disagreements : 0;
}

int CommanderDisagreementSystem::getTotalResolved(const std::string& fleet_id) const {
    const auto* cd = getComponentFor(fleet_id);
    return cd ? cd->total_resolved : 0;
}

std::string CommanderDisagreementSystem::getSeverity(const std::string& fleet_id, int index) const {
    const auto* cd = getComponentFor(fleet_id);
    if (!cd) return "Unknown";

    if (index < 0 || index >= static_cast<int>(cd->disagreements.size())) return "Unknown";
    return components::CommanderDisagreement::severityToString(cd->disagreements[index].severity);
}

std::string CommanderDisagreementSystem::getResolution(const std::string& fleet_id, int index) const {
    const auto* cd = getComponentFor(fleet_id);
    if (!cd) return "None";

    if (index < 0 || index >= static_cast<int>(cd->disagreements.size())) return "None";
    return components::CommanderDisagreement::resolutionToString(cd->disagreements[index].resolution);
}

float CommanderDisagreementSystem::getMoraleImpact(const std::string& fleet_id, int index) const {
    const auto* cd = getComponentFor(fleet_id);
    if (!cd) return 0.0f;

    if (index < 0 || index >= static_cast<int>(cd->disagreements.size())) return 0.0f;
    return cd->disagreements[index].morale_impact;
}

} // namespace systems
} // namespace atlas
