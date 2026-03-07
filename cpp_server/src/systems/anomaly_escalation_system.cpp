#include "systems/anomaly_escalation_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/exploration_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {

using AE = components::AnomalyEscalation;

const char* stateToString(AE::EscalationState s) {
    switch (s) {
        case AE::EscalationState::Idle: return "Idle";
        case AE::EscalationState::SiteActive: return "SiteActive";
        case AE::EscalationState::Cleared: return "Cleared";
        case AE::EscalationState::Escalating: return "Escalating";
        case AE::EscalationState::EscalationReady: return "EscalationReady";
        case AE::EscalationState::Failed: return "Failed";
    }
    return "Unknown";
}

} // anonymous namespace

AnomalyEscalationSystem::AnomalyEscalationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AnomalyEscalationSystem::updateComponent(ecs::Entity& entity,
    components::AnomalyEscalation& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    if (comp.state == AE::EscalationState::Escalating) {
        comp.escalation_timer -= delta_time;
        if (comp.escalation_timer <= 0.0f) {
            comp.escalation_timer = 0.0f;
            comp.state = AE::EscalationState::EscalationReady;
        }
    }
}

bool AnomalyEscalationSystem::initialize(const std::string& entity_id,
    const std::string& system_id, const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AnomalyEscalation>();
    comp->system_id = system_id;
    comp->owner_id = owner_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool AnomalyEscalationSystem::addTier(const std::string& entity_id, int tier,
    const std::string& site_type, float difficulty_multiplier,
    float reward_multiplier, int npc_count) {
    auto* ae = getComponentFor(entity_id);
    if (!ae) return false;
    if (static_cast<int>(ae->tiers.size()) >= ae->max_tiers) return false;
    // Check duplicate tier
    for (const auto& t : ae->tiers) {
        if (t.tier == tier) return false;
    }

    AE::EscalationTier t;
    t.tier = tier;
    t.site_type = site_type;
    t.difficulty_multiplier = difficulty_multiplier;
    t.reward_multiplier = reward_multiplier;
    t.npc_count = npc_count;
    t.completed = false;
    ae->tiers.push_back(t);
    return true;
}

bool AnomalyEscalationSystem::startSite(const std::string& entity_id) {
    auto* ae = getComponentFor(entity_id);
    if (!ae || ae->tiers.empty()) return false;
    if (ae->state != AE::EscalationState::Idle &&
        ae->state != AE::EscalationState::EscalationReady) return false;
    ae->state = AE::EscalationState::SiteActive;
    return true;
}

bool AnomalyEscalationSystem::clearSite(const std::string& entity_id, float roll) {
    auto* ae = getComponentFor(entity_id);
    if (!ae) return false;
    if (ae->state != AE::EscalationState::SiteActive) return false;

    ae->total_sites_cleared++;
    if (ae->current_tier < static_cast<int>(ae->tiers.size())) {
        ae->tiers[ae->current_tier].completed = true;
    }

    // Check if there's a next tier and if escalation triggers
    int next_tier = ae->current_tier + 1;
    if (next_tier < static_cast<int>(ae->tiers.size()) && roll < ae->escalation_chance) {
        ae->current_tier = next_tier;
        ae->escalation_timer = ae->escalation_delay;
        ae->state = AE::EscalationState::Escalating;
        ae->total_escalations_triggered++;
        return true;
    }

    ae->state = AE::EscalationState::Cleared;
    return true;
}

bool AnomalyEscalationSystem::completeEscalation(const std::string& entity_id) {
    auto* ae = getComponentFor(entity_id);
    if (!ae) return false;
    if (ae->state != AE::EscalationState::Cleared &&
        ae->state != AE::EscalationState::EscalationReady &&
        ae->state != AE::EscalationState::SiteActive) return false;
    ae->total_escalations_completed++;
    ae->state = AE::EscalationState::Idle;
    ae->current_tier = 0;
    return true;
}

bool AnomalyEscalationSystem::failEscalation(const std::string& entity_id) {
    auto* ae = getComponentFor(entity_id);
    if (!ae) return false;
    if (ae->state == AE::EscalationState::Idle) return false;
    ae->total_escalations_failed++;
    ae->state = AE::EscalationState::Failed;
    return true;
}

int AnomalyEscalationSystem::getTierCount(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    return ae ? static_cast<int>(ae->tiers.size()) : 0;
}

int AnomalyEscalationSystem::getCurrentTier(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    return ae ? ae->current_tier : -1;
}

std::string AnomalyEscalationSystem::getState(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    if (!ae) return "Unknown";
    return stateToString(ae->state);
}

float AnomalyEscalationSystem::getDifficultyMultiplier(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    if (!ae || ae->tiers.empty()) return 1.0f;
    int idx = std::min(ae->current_tier, static_cast<int>(ae->tiers.size()) - 1);
    return ae->tiers[idx].difficulty_multiplier;
}

float AnomalyEscalationSystem::getRewardMultiplier(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    if (!ae || ae->tiers.empty()) return 1.0f;
    int idx = std::min(ae->current_tier, static_cast<int>(ae->tiers.size()) - 1);
    return ae->tiers[idx].reward_multiplier;
}

int AnomalyEscalationSystem::getTotalSitesCleared(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    return ae ? ae->total_sites_cleared : 0;
}

int AnomalyEscalationSystem::getTotalEscalationsTriggered(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    return ae ? ae->total_escalations_triggered : 0;
}

int AnomalyEscalationSystem::getTotalEscalationsCompleted(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    return ae ? ae->total_escalations_completed : 0;
}

int AnomalyEscalationSystem::getTotalEscalationsFailed(const std::string& entity_id) const {
    auto* ae = getComponentFor(entity_id);
    return ae ? ae->total_escalations_failed : 0;
}

} // namespace systems
} // namespace atlas
