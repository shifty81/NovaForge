#include "systems/galactic_response_curve_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/npc_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

GalacticResponseCurveSystem::GalacticResponseCurveSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void GalacticResponseCurveSystem::updateComponent(ecs::Entity& /*entity*/,
    components::GalacticResponseCurve& grc, float delta_time) {
    if (!grc.active) return;

    // Decay threat over time
    grc.threat_level = std::max(0.0f,
        grc.threat_level - grc.decay_rate * delta_time);

    // Update response tier based on threat level
    if (grc.threat_level >= components::GalacticResponseCurve::TIER_4_THRESHOLD) {
        grc.response_tier = 4;
    } else if (grc.threat_level >= components::GalacticResponseCurve::TIER_3_THRESHOLD) {
        grc.response_tier = 3;
    } else if (grc.threat_level >= components::GalacticResponseCurve::TIER_2_THRESHOLD) {
        grc.response_tier = 2;
    } else if (grc.threat_level >= components::GalacticResponseCurve::TIER_1_THRESHOLD) {
        grc.response_tier = 1;
    } else {
        grc.response_tier = 0;
    }
}

bool GalacticResponseCurveSystem::initializeFaction(const std::string& entity_id,
    const std::string& faction_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::GalacticResponseCurve>();
    comp->faction_id = faction_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool GalacticResponseCurveSystem::reportThreat(const std::string& entity_id,
    float magnitude) {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return false;
    grc->threat_level += magnitude * grc->escalation_rate;
    grc->threat_level = std::max(0.0f, grc->threat_level);
    return true;
}

bool GalacticResponseCurveSystem::dispatchReinforcement(const std::string& entity_id) {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return false;
    // Must be at tier 3+ to dispatch reinforcements
    if (grc->response_tier < 3) return false;
    grc->reinforcements_dispatched++;
    return true;
}

bool GalacticResponseCurveSystem::rerouteTradeFor(const std::string& entity_id,
    const std::string& system_id) {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return false;
    if (static_cast<int>(grc->rerouted_systems.size()) >= grc->max_rerouted) return false;

    // Check for duplicate
    for (const auto& s : grc->rerouted_systems) {
        if (s == system_id) return false;
    }

    grc->rerouted_systems.push_back(system_id);
    return true;
}

float GalacticResponseCurveSystem::getThreatLevel(const std::string& entity_id) const {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return 0.0f;
    return grc->threat_level;
}

int GalacticResponseCurveSystem::getResponseTier(const std::string& entity_id) const {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return 0;
    return grc->response_tier;
}

int GalacticResponseCurveSystem::getReinforcementCount(const std::string& entity_id) const {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return 0;
    return grc->reinforcements_dispatched;
}

int GalacticResponseCurveSystem::getReroutedSystemCount(const std::string& entity_id) const {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return 0;
    return static_cast<int>(grc->rerouted_systems.size());
}

float GalacticResponseCurveSystem::getEscalationRate(const std::string& entity_id) const {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return 0.0f;
    return grc->escalation_rate;
}

bool GalacticResponseCurveSystem::isFullMobilization(const std::string& entity_id) const {
    auto* grc = getComponentFor(entity_id);
    if (!grc) return false;
    return grc->response_tier >= 4;
}

} // namespace systems
} // namespace atlas
