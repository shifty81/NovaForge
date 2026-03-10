#include "systems/incursion_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

IncursionSystem::IncursionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void IncursionSystem::updateComponent(ecs::Entity& entity,
    components::IncursionState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Transition from Spawning → Active on first tick
    if (comp.lifecycle == components::IncursionState::Lifecycle::Spawning) {
        comp.lifecycle = components::IncursionState::Lifecycle::Active;
    }

    // Slow influence decay over time
    if (comp.lifecycle == components::IncursionState::Lifecycle::Active) {
        comp.influence -= comp.influence_decay_rate * delta_time;
        comp.influence = (std::max)(comp.influence, 0.0f);

        // Withdrawal check
        if (comp.influence <= 0.0f) {
            comp.lifecycle = components::IncursionState::Lifecycle::Withdrawn;
            comp.active = false;
        }
    }
}

bool IncursionSystem::initialize(const std::string& entity_id,
    const std::string& constellation_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::IncursionState>();
    comp->constellation_id = constellation_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool IncursionSystem::spawnSite(const std::string& entity_id,
    const std::string& site_id, components::IncursionState::Tier tier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->sites.size()) >= comp->max_sites) return false;

    components::IncursionState::IncursionSite site;
    site.site_id = site_id;
    site.tier = tier;
    // Wave count scales with tier
    switch (tier) {
        case components::IncursionState::Tier::Vanguard:
            site.max_waves = 3;
            break;
        case components::IncursionState::Tier::Assault:
            site.max_waves = 5;
            break;
        case components::IncursionState::Tier::Headquarters:
            site.max_waves = 7;
            break;
    }
    comp->sites.push_back(site);
    return true;
}

bool IncursionSystem::completeSite(const std::string& entity_id,
    const std::string& site_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& site : comp->sites) {
        if (site.site_id == site_id && !site.completed) {
            site.completed = true;
            comp->completed_sites++;

            // Count fleet members in this site for reward scaling
            int fleet_in_site = 0;
            for (const auto& fm : comp->fleet_members) {
                if (fm.site_id == site_id) fleet_in_site++;
            }
            int fleet_bonus = (std::max)(fleet_in_site, 1);

            int base_lp = components::IncursionState::baseLPForTier(site.tier);
            int payout = base_lp * fleet_bonus;
            site.lp_reward = payout;
            comp->total_lp_paid += static_cast<float>(payout);

            // Reduce influence based on tier
            float influence_reduction = 0.0f;
            switch (site.tier) {
                case components::IncursionState::Tier::Vanguard:
                    influence_reduction = 1.0f;
                    break;
                case components::IncursionState::Tier::Assault:
                    influence_reduction = 3.0f;
                    break;
                case components::IncursionState::Tier::Headquarters:
                    influence_reduction = 7.0f;
                    break;
            }
            comp->influence = (std::max)(comp->influence - influence_reduction, 0.0f);

            if (comp->influence <= 0.0f) {
                comp->lifecycle = components::IncursionState::Lifecycle::Withdrawn;
                comp->active = false;
            }
            return true;
        }
    }
    return false;
}

bool IncursionSystem::registerFleetMember(const std::string& entity_id,
    const std::string& site_id, const std::string& pilot_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    components::IncursionState::FleetMember fm;
    fm.pilot_id = pilot_id;
    fm.site_id = site_id;
    comp->fleet_members.push_back(fm);
    return true;
}

bool IncursionSystem::applyInfluenceDelta(const std::string& entity_id,
    float delta) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->influence = (std::min)((std::max)(comp->influence + delta, 0.0f), 100.0f);

    if (comp->influence <= 0.0f) {
        comp->lifecycle = components::IncursionState::Lifecycle::Withdrawn;
        comp->active = false;
    }
    return true;
}

int IncursionSystem::getSiteCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->sites.size()) : 0;
}

float IncursionSystem::getInfluence(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->influence : 0.0f;
}

bool IncursionSystem::isWithdrawn(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->lifecycle == components::IncursionState::Lifecycle::Withdrawn) : false;
}

int IncursionSystem::getCompletedSiteCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->completed_sites : 0;
}

float IncursionSystem::getTotalLPPaid(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_lp_paid : 0.0f;
}

int IncursionSystem::getFleetSize(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->fleet_members.size()) : 0;
}

} // namespace systems
} // namespace atlas
