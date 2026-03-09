#include "systems/ai_system.h"
#include "systems/combat_system.h"
#include "ecs/world.h"
#include "components/game_components.h"
#include <cmath>
#include <limits>
#include <algorithm>

namespace atlas {
namespace systems {

AISystem::AISystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void AISystem::updateComponent(ecs::Entity& entity, components::AI& ai, float /*delta_time*/) {
    // Require Position and Velocity (same filter as original multi-component query)
    if (!entity.getComponent<components::Position>()) return;
    if (!entity.getComponent<components::Velocity>()) return;
    
    // Execute behavior based on current state
    switch (ai.state) {
        case components::AI::State::Idle:
            idleBehavior(&entity);
            break;
        case components::AI::State::Approaching:
            approachBehavior(&entity);
            break;
        case components::AI::State::Orbiting:
            orbitBehavior(&entity);
            break;
        case components::AI::State::Attacking:
            attackBehavior(&entity);
            break;
        case components::AI::State::Fleeing:
            fleeBehavior(&entity);
            break;
        case components::AI::State::Mining:
            miningBehavior(&entity);
            break;
        case components::AI::State::Hauling:
            haulingBehavior(&entity);
            break;
    }
}

void AISystem::idleBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    
    if (!ai || !pos) return;
    
    // Mining NPCs (passive with a MiningLaser) look for deposits
    if (ai->behavior == components::AI::Behavior::Passive) {
        auto* laser = entity->getComponent<components::MiningLaser>();
        if (laser) {
            ecs::Entity* deposit = findMostProfitableDeposit(entity);
            if (deposit) {
                ai->target_entity_id = deposit->getId();
                ai->state = components::AI::State::Approaching;
                return;
            }
        }
    }
    
    // Defensive NPCs protect nearby friendly entities under attack
    if (ai->behavior == components::AI::Behavior::Defensive) {
        ecs::Entity* attacker = findAttackerOfFriendly(entity);
        if (attacker) {
            ai->target_entity_id = attacker->getId();
            ai->state = components::AI::State::Approaching;
            return;
        }
    }
    
    // Only aggressive NPCs actively seek targets
    if (ai->behavior != components::AI::Behavior::Aggressive) {
        return;
    }
    
    // Apply dynamic orbit distance from ship class if enabled
    if (ai->use_dynamic_orbit) {
        auto* ship = entity->getComponent<components::Ship>();
        if (ship) {
            ai->orbit_distance = orbitDistanceForClass(ship->ship_class);
        }
    }
    
    // Derive engagement range from weapon if not set
    if (ai->engagement_range <= 0.0f) {
        ai->engagement_range = engagementRangeFromWeapon(entity);
    }
    
    // Use target selection strategy
    ecs::Entity* target = selectTarget(entity);
    
    // If found a target, switch to approaching
    if (target) {
        ai->target_entity_id = target->getId();
        ai->state = components::AI::State::Approaching;
    }
}

void AISystem::approachBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* vel = entity->getComponent<components::Velocity>();
    
    if (!ai || !pos || !vel) return;
    
    // Check if target still exists
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* target_pos = target->getComponent<components::Position>();
    if (!target_pos) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Calculate distance to target
    float dx = target_pos->x - pos->x;
    float dy = target_pos->y - pos->y;
    float dz = target_pos->z - pos->z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    // Use engagement range to decide when to transition.
    // If engagement_range is set and we're within it, start orbiting.
    // Otherwise fall back to orbit_distance.
    float engage = (ai->engagement_range > 0.0f) ? ai->engagement_range : ai->orbit_distance;
    
    // Check if target is a mineral deposit — transition to Mining instead of Orbiting
    if (target->hasComponent<components::MineralDeposit>()) {
        float mining_range = 10000.0f;  // default mining range
        if (distance < mining_range) {
            ai->state = components::AI::State::Mining;
            return;
        }
    } else if (distance < std::min(engage, ai->orbit_distance)) {
        ai->state = components::AI::State::Orbiting;
        return;
    }
    
    // Move towards target
    if (distance > 0.0f) {
        vel->vx = (dx / distance) * vel->max_speed;
        vel->vy = (dy / distance) * vel->max_speed;
        vel->vz = (dz / distance) * vel->max_speed;
    }
}

void AISystem::orbitBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* vel = entity->getComponent<components::Velocity>();
    
    if (!ai || !pos || !vel) return;
    
    // Check if target still exists
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* target_pos = target->getComponent<components::Position>();
    if (!target_pos) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Calculate perpendicular velocity for orbiting
    float dx = target_pos->x - pos->x;
    float dy = target_pos->y - pos->y;
    float distance = std::sqrt(dx * dx + dy * dy);
    
    if (distance > 0.0f) {
        // Perpendicular velocity for orbiting (2D orbit in xy plane)
        vel->vx = -(dy / distance) * vel->max_speed * 0.5f;
        vel->vy = (dx / distance) * vel->max_speed * 0.5f;
        vel->vz = 0.0f;
    }
    
    // Switch to attacking if we have weapons
    if (entity->hasComponent<components::Weapon>()) {
        ai->state = components::AI::State::Attacking;
    }
}

void AISystem::attackBehavior(ecs::Entity* entity) {
    // Check health for flee decision
    auto* ai = entity->getComponent<components::AI>();
    auto* health = entity->getComponent<components::Health>();
    if (ai && health) {
        float total_max = health->shield_max + health->armor_max + health->hull_max;
        float total_current = health->shield_hp + health->armor_hp + health->hull_hp;
        if (total_max > 0.0f && (total_current / total_max) < ai->flee_threshold) {
            ai->state = components::AI::State::Fleeing;
            return;
        }
    }
    
    // Continue orbiting while attacking
    orbitBehavior(entity);
    
    // Weapon firing is handled by CombatSystem
    // The AI just maintains the target and orbit
}

void AISystem::fleeBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* vel = entity->getComponent<components::Velocity>();
    
    if (!ai || !pos || !vel) return;
    
    // Check if target still exists
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* target_pos = target->getComponent<components::Position>();
    if (!target_pos) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Move away from target
    float dx = pos->x - target_pos->x;
    float dy = pos->y - target_pos->y;
    float dz = pos->z - target_pos->z;
    float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
    
    if (distance > 0.0f) {
        vel->vx = (dx / distance) * vel->max_speed;
        vel->vy = (dy / distance) * vel->max_speed;
        vel->vz = (dz / distance) * vel->max_speed;
    }
    
    // If far enough away, switch back to idle
    if (distance > ai->awareness_range) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
    }
}

ecs::Entity* AISystem::selectTarget(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    if (!ai || !pos) return nullptr;
    
    auto all_entities = world_->getEntities<components::Position>();
    
    ecs::Entity* best_target = nullptr;
    float best_score = (std::numeric_limits<float>::max)();
    
    // Get our faction for standing checks
    auto* our_faction = entity->getComponent<components::Faction>();
    
    for (auto* candidate : all_entities) {
        if (candidate == entity) continue;
        if (!candidate->hasComponent<components::Player>() &&
            !candidate->hasComponent<components::AI>()) continue;
        
        // Skip entities with positive faction standing (friendly)
        if (our_faction) {
            auto* their_standings = candidate->getComponent<components::Standings>();
            auto* their_faction = candidate->getComponent<components::Faction>();
            if (their_standings) {
                float standing = their_standings->getStandingWith(
                    entity->getId(), "",
                    our_faction->faction_name);
                if (standing > 0.0f) continue;  // friendly — do not target
            } else if (their_faction) {
                // Check faction-to-faction standing
                auto it = our_faction->standings.find(their_faction->faction_name);
                if (it != our_faction->standings.end() && it->second > 0.0f) continue;
            }
        }
        
        auto* target_pos = candidate->getComponent<components::Position>();
        if (!target_pos) continue;
        
        float dx = target_pos->x - pos->x;
        float dy = target_pos->y - pos->y;
        float dz = target_pos->z - pos->z;
        float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        if (distance > ai->awareness_range) continue;
        
        float score = 0.0f;
        
        switch (ai->target_selection) {
            case components::AI::TargetSelection::Closest:
                score = distance;
                break;
                
            case components::AI::TargetSelection::LowestHP: {
                auto* health = candidate->getComponent<components::Health>();
                if (health) {
                    float total_max = health->shield_max + health->armor_max + health->hull_max;
                    float total_current = health->shield_hp + health->armor_hp + health->hull_hp;
                    score = (total_max > 0.0f) ? (total_current / total_max) : 1.0f;
                } else {
                    score = 1.0f;
                }
                break;
            }
            
            case components::AI::TargetSelection::HighestThreat: {
                auto* dmg = entity->getComponent<components::DamageEvent>();
                // Lower score = higher priority; invert damage to make most-damaging a lower score
                float threat = 0.0f;
                if (dmg) {
                    for (const auto& hit : dmg->recent_hits) {
                        threat += hit.damage_amount;
                    }
                }
                // Use negative threat as score so higher threat = lower score = higher priority
                // Add distance as tiebreaker
                score = -threat + distance * 0.001f;
                break;
            }
        }
        
        if (score < best_score) {
            best_score = score;
            best_target = candidate;
        }
    }
    
    return best_target;
}

float AISystem::orbitDistanceForClass(const std::string& ship_class) {
    if (ship_class == "Frigate" || ship_class == "Destroyer") {
        return 5000.0f;
    } else if (ship_class == "Cruiser" || ship_class == "Battlecruiser") {
        return 15000.0f;
    } else if (ship_class == "Battleship") {
        return 30000.0f;
    } else if (ship_class == "Capital" || ship_class == "Carrier" ||
               ship_class == "Dreadnought" || ship_class == "Titan") {
        return 50000.0f;
    }
    return 10000.0f;  // default for unknown classes
}

float AISystem::engagementRangeFromWeapon(ecs::Entity* entity) {
    auto* weapon = entity->getComponent<components::Weapon>();
    if (!weapon) return 0.0f;
    return weapon->optimal_range + weapon->falloff_range;
}

void AISystem::miningBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    
    if (!ai || !pos) return;
    
    // Check if target deposit still exists and is minable
    if (ai->target_entity_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }
    
    auto* target = world_->getEntity(ai->target_entity_id);
    if (!target) {
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    auto* deposit = target->getComponent<components::MineralDeposit>();
    if (!deposit || deposit->isDepleted()) {
        // Deposit exhausted — return to idle to find a new one
        ai->state = components::AI::State::Idle;
        ai->target_entity_id.clear();
        return;
    }
    
    // Check cargo capacity
    auto* inv = entity->getComponent<components::Inventory>();
    if (inv && inv->freeCapacity() <= 0.0f) {
        // Cargo full — deactivate laser and transition to hauling if station set
        auto* ml = entity->getComponent<components::MiningLaser>();
        if (ml) {
            ml->active = false;
        }
        if (!ai->haul_station_id.empty()) {
            ai->state = components::AI::State::Hauling;
        } else {
            ai->state = components::AI::State::Idle;
        }
        ai->target_entity_id.clear();
        return;
    }
    
    // Activate mining laser if not already active
    auto* laser = entity->getComponent<components::MiningLaser>();
    if (laser && !laser->active) {
        laser->active = true;
        laser->cycle_progress = 0.0f;
        laser->target_deposit_id = ai->target_entity_id;
    }
}

ecs::Entity* AISystem::findNearestDeposit(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    if (!ai || !pos) return nullptr;
    
    auto all_entities = world_->getEntities<components::Position, components::MineralDeposit>();
    
    ecs::Entity* nearest = nullptr;
    float best_dist = (std::numeric_limits<float>::max)();
    
    for (auto* candidate : all_entities) {
        auto* dep = candidate->getComponent<components::MineralDeposit>();
        if (!dep || dep->isDepleted()) continue;
        
        auto* dep_pos = candidate->getComponent<components::Position>();
        if (!dep_pos) continue;
        
        float dx = dep_pos->x - pos->x;
        float dy = dep_pos->y - pos->y;
        float dz = dep_pos->z - pos->z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        
        if (dist > ai->awareness_range) continue;
        
        if (dist < best_dist) {
            best_dist = dist;
            nearest = candidate;
        }
    }
    
    return nearest;
}

ecs::Entity* AISystem::findMostProfitableDeposit(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    if (!ai || !pos) return nullptr;

    // Look for a SupplyDemand entity to get market prices
    auto sd_entities = world_->getEntities<components::SupplyDemand>();
    components::SupplyDemand* sd = nullptr;
    for (auto* e : sd_entities) {
        sd = e->getComponent<components::SupplyDemand>();
        if (sd) break;
    }

    // If no market data, fall back to nearest deposit
    if (!sd || sd->commodities.empty()) {
        return findNearestDeposit(entity);
    }

    auto all_entities = world_->getEntities<components::Position, components::MineralDeposit>();

    ecs::Entity* best = nullptr;
    float best_score = -1.0f;

    for (auto* candidate : all_entities) {
        auto* dep = candidate->getComponent<components::MineralDeposit>();
        if (!dep || dep->isDepleted()) continue;

        auto* dep_pos = candidate->getComponent<components::Position>();
        if (!dep_pos) continue;

        float dx = dep_pos->x - pos->x;
        float dy = dep_pos->y - pos->y;
        float dz = dep_pos->z - pos->z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist > ai->awareness_range) continue;
        if (dist < 1.0f) dist = 1.0f;  // avoid division by zero

        // Look up market price for this mineral type
        float price = 0.0f;
        const auto* commodity = sd->getCommodity(dep->mineral_type);
        if (commodity) {
            price = commodity->current_price;
        }

        // Score = price / distance  (higher is better)
        float score = price / dist;
        if (score > best_score) {
            best_score = score;
            best = candidate;
        }
    }

    // If no scored deposits found (all prices zero), fall back to nearest
    if (!best) {
        return findNearestDeposit(entity);
    }

    return best;
}

void AISystem::haulingBehavior(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();

    if (!ai || !pos) return;

    // Need a station to haul to
    if (ai->haul_station_id.empty()) {
        ai->state = components::AI::State::Idle;
        return;
    }

    auto* station = world_->getEntity(ai->haul_station_id);
    if (!station) {
        ai->state = components::AI::State::Idle;
        ai->haul_station_id.clear();
        return;
    }

    auto* station_pos = station->getComponent<components::Position>();
    if (!station_pos) {
        ai->state = components::AI::State::Idle;
        return;
    }

    float dx = station_pos->x - pos->x;
    float dy = station_pos->y - pos->y;
    float dz = station_pos->z - pos->z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    const float docking_range = 500.0f;

    if (dist > docking_range) {
        // Move toward station
        auto* vel = entity->getComponent<components::Velocity>();
        if (vel && dist > 0.0f) {
            float speed = vel->max_speed;
            vel->vx = (dx / dist) * speed;
            vel->vy = (dy / dist) * speed;
            vel->vz = (dz / dist) * speed;
        }
        return;
    }

    // Within docking range — sell cargo
    auto* inv = entity->getComponent<components::Inventory>();
    auto* intent = entity->getComponent<components::SimNPCIntent>();
    if (inv && !inv->items.empty()) {
        // Look for SupplyDemand to price the ore
        auto sd_entities = world_->getEntities<components::SupplyDemand>();
        components::SupplyDemand* sd = nullptr;
        for (auto* e : sd_entities) {
            sd = e->getComponent<components::SupplyDemand>();
            if (sd) break;
        }

        double total_earnings = 0.0;
        for (const auto& item : inv->items) {
            float unit_price = 0.0f;
            if (sd) {
                const auto* commodity = sd->getCommodity(item.item_id);
                if (commodity) {
                    unit_price = commodity->current_price;
                }
            }
            total_earnings += static_cast<double>(unit_price) * item.quantity;
        }

        if (intent) {
            intent->wallet += total_earnings;
        }

        inv->items.clear();
    }

    // Stop moving and return to idle to mine again
    auto* vel = entity->getComponent<components::Velocity>();
    if (vel) {
        vel->vx = 0.0f;
        vel->vy = 0.0f;
        vel->vz = 0.0f;
    }
    ai->state = components::AI::State::Idle;
}

ecs::Entity* AISystem::findAttackerOfFriendly(ecs::Entity* entity) {
    auto* ai = entity->getComponent<components::AI>();
    auto* pos = entity->getComponent<components::Position>();
    auto* our_faction = entity->getComponent<components::Faction>();
    if (!ai || !pos || !our_faction) return nullptr;

    auto candidates = world_->getEntities<components::Position, components::DamageEvent>();

    for (auto* friendly : candidates) {
        if (friendly == entity) continue;

        auto* f_pos = friendly->getComponent<components::Position>();
        if (!f_pos) continue;

        float dx = f_pos->x - pos->x;
        float dy = f_pos->y - pos->y;
        float dz = f_pos->z - pos->z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);
        if (dist > ai->awareness_range) continue;

        // Check if this entity is friendly to us
        auto* their_standings = friendly->getComponent<components::Standings>();
        auto* their_faction = friendly->getComponent<components::Faction>();
        bool is_friendly = false;
        if (their_standings) {
            float standing = their_standings->getStandingWith(
                entity->getId(), "", our_faction->faction_name);
            is_friendly = (standing > 0.0f);
        } else if (their_faction) {
            auto it = our_faction->standings.find(their_faction->faction_name);
            if (it != our_faction->standings.end()) {
                is_friendly = (it->second > 0.0f);
            }
        }

        if (!is_friendly) continue;

        // This entity is friendly and has damage events — find who is attacking them
        auto* dmg = friendly->getComponent<components::DamageEvent>();
        if (!dmg || dmg->recent_hits.empty()) continue;

        // The most recent hit's source is the attacker
        // DamageEvent doesn't store attacker id, so look for nearby hostiles
        // targeting this friendly entity
        auto all_ai = world_->getEntities<components::AI, components::Position>();
        for (auto* potential_attacker : all_ai) {
            if (potential_attacker == entity) continue;
            auto* atk_ai = potential_attacker->getComponent<components::AI>();
            if (!atk_ai) continue;
            if (atk_ai->target_entity_id != friendly->getId()) continue;

            // Confirm the attacker is hostile to us
            auto* atk_faction = potential_attacker->getComponent<components::Faction>();
            if (atk_faction) {
                auto it = our_faction->standings.find(atk_faction->faction_name);
                if (it != our_faction->standings.end() && it->second > 0.0f) continue;
            }

            return potential_attacker;
        }
    }

    return nullptr;
}

} // namespace systems
} // namespace atlas
