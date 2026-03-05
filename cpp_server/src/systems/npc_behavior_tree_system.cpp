#include "systems/npc_behavior_tree_system.h"
#include "ecs/world.h"
#include <algorithm>

namespace atlas {
namespace systems {

NPCBehaviorTreeSystem::NPCBehaviorTreeSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void NPCBehaviorTreeSystem::updateComponent(ecs::Entity& entity, components::NPCBehaviorState& behavior, float delta_time) {
    auto* intent = entity.getComponent<components::SimNPCIntent>();
    if (!intent) return;

    tickBehavior(&entity, &behavior, intent, delta_time);
}

// -----------------------------------------------------------------------
// Per-entity behavior tree tick
// -----------------------------------------------------------------------

void NPCBehaviorTreeSystem::tickBehavior(ecs::Entity* /*entity*/,
                                          components::NPCBehaviorState* behavior,
                                          components::SimNPCIntent* intent,
                                          float dt) {
    // If intent changed since last tick, rebuild phase list
    if (intent->current_intent != behavior->bound_intent) {
        auto phases = getPhasesForIntent(intent->archetype, intent->current_intent);
        behavior->phases = phases;
        behavior->current_phase_index = 0;
        behavior->phase_elapsed = 0.0f;
        behavior->tree_complete = false;
        behavior->bound_intent = intent->current_intent;
    }

    if (behavior->tree_complete || behavior->phases.empty()) return;

    behavior->phase_elapsed += dt;

    if (shouldAdvancePhase(behavior, intent)) {
        behavior->current_phase_index++;
        behavior->phase_elapsed = 0.0f;

        if (behavior->current_phase_index >= static_cast<int>(behavior->phases.size())) {
            behavior->tree_complete = true;
            intent->intent_complete = true;
        }
    }
}

// -----------------------------------------------------------------------
// Phase advancement heuristic — phases advance after a minimum duration
// -----------------------------------------------------------------------

bool NPCBehaviorTreeSystem::shouldAdvancePhase(
        const components::NPCBehaviorState* behavior,
        const components::SimNPCIntent* /*intent*/) const {
    return behavior->phase_elapsed >= behavior->phase_duration;
}

// -----------------------------------------------------------------------
// Static phase tables per archetype / intent
// -----------------------------------------------------------------------

std::vector<std::string> NPCBehaviorTreeSystem::getPhasesForIntent(
        components::SimNPCIntent::Archetype archetype,
        components::SimNPCIntent::Intent intent) {
    using A = components::SimNPCIntent::Archetype;
    using I = components::SimNPCIntent::Intent;

    // Default generic phases
    auto generic = [&]() -> std::vector<std::string> {
        switch (intent) {
        case I::Trade:   return {"FindGoods", "TravelToMarket", "SellGoods"};
        case I::Patrol:  return {"PickWaypoint", "TravelTo", "ScanArea"};
        case I::Hunt:    return {"SearchTargets", "Approach", "Engage", "Loot"};
        case I::Explore: return {"SelectDestination", "TravelTo", "ScanSite"};
        case I::Flee:    return {"SelectSafespot", "Warp", "HoldPosition"};
        case I::Escort:  return {"FormOnLeader", "MatchSpeed", "Defend"};
        case I::Salvage: return {"FindWreck", "TravelTo", "SalvageWreck"};
        case I::Mine:    return {"FindDeposit", "MineOre", "HaulToStation"};
        case I::Haul:    return {"PickUpCargo", "TravelTo", "DeliverCargo"};
        case I::Dock:    return {"TravelToStation", "RequestDock", "Dock"};
        case I::Idle:
        default:         return {"Wait"};
        }
    };

    // Archetype-specific overrides where behaviour differs
    if (archetype == A::Trader && intent == I::Trade) {
        return {"CheckMarketPrices", "BuyGoods", "TravelToMarket", "SellGoods", "EvaluateProfit"};
    }
    if (archetype == A::Miner && intent == I::Mine) {
        return {"FindDeposit", "ApproachRock", "ActivateLasers", "FillCargo", "HaulToStation"};
    }
    if (archetype == A::Pirate && intent == I::Hunt) {
        return {"ScanForPrey", "Stalk", "Ambush", "Engage", "LootWreck"};
    }
    if (archetype == A::Patrol && intent == I::Patrol) {
        return {"PickWaypoint", "TravelTo", "ScanArea", "ReportStatus"};
    }
    if (archetype == A::Hauler && intent == I::Haul) {
        return {"AcceptContract", "PickUpCargo", "TravelTo", "DeliverCargo", "CollectPayment"};
    }
    if (archetype == A::Industrialist && intent == I::Trade) {
        return {"CheckSupply", "ManufactureGoods", "TravelToMarket", "SellGoods"};
    }

    return generic();
}

// -----------------------------------------------------------------------
// Query API
// -----------------------------------------------------------------------

std::string NPCBehaviorTreeSystem::getCurrentPhase(const std::string& entity_id) const {
    const auto* behavior = getComponentFor(entity_id);
    if (!behavior || behavior->phases.empty()) return "";

    int idx = std::min(behavior->current_phase_index,
                       static_cast<int>(behavior->phases.size()) - 1);
    return behavior->phases[static_cast<size_t>(idx)];
}

float NPCBehaviorTreeSystem::getPhaseElapsed(const std::string& entity_id) const {
    const auto* behavior = getComponentFor(entity_id);
    if (!behavior) return 0.0f;

    return behavior->phase_elapsed;
}

bool NPCBehaviorTreeSystem::isTreeComplete(const std::string& entity_id) const {
    const auto* behavior = getComponentFor(entity_id);
    if (!behavior) return true;

    return behavior->tree_complete;
}

void NPCBehaviorTreeSystem::resetTree(const std::string& entity_id) {
    auto* behavior = getComponentFor(entity_id);
    if (!behavior) return;

    behavior->current_phase_index = 0;
    behavior->phase_elapsed = 0.0f;
    behavior->tree_complete = false;
}

} // namespace systems
} // namespace atlas
