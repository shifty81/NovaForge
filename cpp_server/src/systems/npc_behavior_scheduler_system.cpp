#include "systems/npc_behavior_scheduler_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

NPCBehaviorSchedulerSystem::NPCBehaviorSchedulerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NPCBehaviorSchedulerSystem::updateComponent(ecs::Entity& /*entity*/,
    components::NPCBehaviorSchedulerState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    using Behavior = components::NPCBehaviorSchedulerState::Behavior;

    // Threat override: high threat forces combat or flee
    if (comp.threat_level >= comp.threat_threshold) {
        int forced = static_cast<int>(Behavior::Combat);
        if (comp.threat_level > 0.8f) {
            forced = static_cast<int>(Behavior::Flee);
        }
        if (comp.current_behavior != forced) {
            comp.previous_behavior = comp.current_behavior;
            comp.current_behavior = forced;
            comp.total_transitions++;
            if (forced == static_cast<int>(Behavior::Combat))
                comp.total_combat_entries++;
        }
        return;
    }

    // Normal schedule-based behavior
    for (const auto& entry : comp.schedule) {
        float end_hour = entry.start_hour + entry.duration_hours;
        bool in_window = false;
        if (end_hour <= 24.0f) {
            in_window = comp.current_game_hour >= entry.start_hour &&
                        comp.current_game_hour < end_hour;
        } else {
            // Wraps past midnight
            in_window = comp.current_game_hour >= entry.start_hour ||
                        comp.current_game_hour < (end_hour - 24.0f);
        }

        if (in_window && comp.current_behavior != entry.behavior) {
            comp.previous_behavior = comp.current_behavior;
            comp.current_behavior = entry.behavior;
            comp.total_transitions++;
            if (entry.behavior == static_cast<int>(Behavior::Trade))
                comp.total_trade_trips++;
            if (entry.behavior == static_cast<int>(Behavior::Combat))
                comp.total_combat_entries++;
            break;
        }
    }
}

bool NPCBehaviorSchedulerSystem::initialize(const std::string& entity_id,
    const std::string& npc_id, const std::string& faction) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (npc_id.empty() || faction.empty()) return false;

    auto comp = std::make_unique<components::NPCBehaviorSchedulerState>();
    comp->npc_id = npc_id;
    comp->faction = faction;
    entity->addComponent(std::move(comp));
    return true;
}

bool NPCBehaviorSchedulerSystem::addScheduleEntry(const std::string& entity_id,
    const std::string& label, int behavior, float start_hour, float duration_hours) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (label.empty()) return false;
    if (behavior < 0 || behavior > 6) return false;
    if (start_hour < 0.0f || start_hour >= 24.0f) return false;
    if (duration_hours <= 0.0f || duration_hours > 24.0f) return false;

    // Check for duplicate label
    for (const auto& e : comp->schedule) {
        if (e.label == label) return false;
    }
    if (static_cast<int>(comp->schedule.size()) >= comp->max_schedule_entries)
        return false;

    components::NPCBehaviorSchedulerState::ScheduleEntry entry;
    entry.label = label;
    entry.behavior = behavior;
    entry.start_hour = start_hour;
    entry.duration_hours = duration_hours;
    comp->schedule.push_back(entry);
    return true;
}

bool NPCBehaviorSchedulerSystem::removeScheduleEntry(const std::string& entity_id,
    const std::string& label) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->schedule.begin(), comp->schedule.end(),
        [&label](const components::NPCBehaviorSchedulerState::ScheduleEntry& e) {
            return e.label == label;
        });
    if (it == comp->schedule.end()) return false;
    comp->schedule.erase(it);
    return true;
}

bool NPCBehaviorSchedulerSystem::clearSchedule(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->schedule.clear();
    return true;
}

bool NPCBehaviorSchedulerSystem::setGameHour(const std::string& entity_id, float hour) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (hour < 0.0f || hour >= 24.0f) return false;
    comp->current_game_hour = hour;
    return true;
}

bool NPCBehaviorSchedulerSystem::setThreatLevel(const std::string& entity_id, float threat) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threat < 0.0f || threat > 1.0f) return false;
    comp->threat_level = threat;
    return true;
}

bool NPCBehaviorSchedulerSystem::setThreatThreshold(const std::string& entity_id,
    float threshold) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (threshold < 0.0f || threshold > 1.0f) return false;
    comp->threat_threshold = threshold;
    return true;
}

bool NPCBehaviorSchedulerSystem::forceBehavior(const std::string& entity_id, int behavior) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (behavior < 0 || behavior > 6) return false;

    comp->previous_behavior = comp->current_behavior;
    comp->current_behavior = behavior;
    comp->total_transitions++;
    return true;
}

int NPCBehaviorSchedulerSystem::getCurrentBehavior(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_behavior : 0;
}

int NPCBehaviorSchedulerSystem::getPreviousBehavior(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->previous_behavior : 0;
}

int NPCBehaviorSchedulerSystem::getScheduleEntryCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->schedule.size()) : 0;
}

float NPCBehaviorSchedulerSystem::getGameHour(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_game_hour : 0.0f;
}

float NPCBehaviorSchedulerSystem::getThreatLevel(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->threat_level : 0.0f;
}

int NPCBehaviorSchedulerSystem::getTotalTransitions(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_transitions : 0;
}

int NPCBehaviorSchedulerSystem::getTotalCombatEntries(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_combat_entries : 0;
}

int NPCBehaviorSchedulerSystem::getTotalTradeTrips(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_trade_trips : 0;
}

std::string NPCBehaviorSchedulerSystem::getFaction(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->faction : "";
}

} // namespace systems
} // namespace atlas
