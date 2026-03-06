#include "systems/npc_schedule_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/npc_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using Sched = components::NPCSchedule;
using Entry = components::NPCSchedule::ScheduleEntry;
using Activity = components::NPCSchedule::Activity;

// Find the best matching schedule entry for the given hour.
// Returns the highest-priority entry whose time window contains the hour.
const Entry* findActiveEntry(const Sched* s, float hour) {
    const Entry* best = nullptr;
    for (const auto& e : s->schedule) {
        bool in_range = false;
        if (e.start_hour <= e.end_hour) {
            in_range = (hour >= e.start_hour && hour < e.end_hour);
        } else {
            // Wrapping range (e.g., 22:00 to 06:00)
            in_range = (hour >= e.start_hour || hour < e.end_hour);
        }
        if (in_range) {
            if (!best || e.priority > best->priority) {
                best = &e;
            }
        }
    }
    return best;
}

} // anonymous namespace

NPCScheduleSystem::NPCScheduleSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NPCScheduleSystem::updateComponent(ecs::Entity& entity,
    components::NPCSchedule& sched, float delta_time) {
    if (!sched.active) return;

    // Advance day time
    sched.elapsed_day_time += delta_time;
    if (sched.elapsed_day_time >= sched.day_length) {
        sched.elapsed_day_time -= sched.day_length;
        sched.days_completed++;
    }

    // Convert elapsed time to hour of day (0-24)
    sched.current_hour = (sched.elapsed_day_time / sched.day_length) * 24.0f;

    // Find the scheduled activity for current hour
    const Entry* active = findActiveEntry(&sched, sched.current_hour);
    Activity target = active ? active->activity : Activity::Idle;

    if (target != sched.current_activity) {
        sched.current_activity = target;
        sched.transitions++;
    }
}

bool NPCScheduleSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NPCSchedule>();
    entity->addComponent(std::move(comp));
    return true;
}

bool NPCScheduleSystem::addEntry(const std::string& entity_id, int activity,
    float start_hour, float end_hour, const std::string& location, int priority) {
    auto* sched = getComponentFor(entity_id);
    if (!sched) return false;
    if (static_cast<int>(sched->schedule.size()) >= sched->max_entries) return false;
    if (activity < 0 || activity > 6) return false;
    if (priority < 1 || priority > 5) return false;

    Entry e;
    e.activity = static_cast<Activity>(activity);
    e.start_hour = std::fmod(std::max(0.0f, start_hour), 24.0f);
    e.end_hour = std::fmod(std::max(0.0f, end_hour), 24.0f);
    e.location = location;
    e.priority = priority;
    sched->schedule.push_back(e);
    return true;
}

bool NPCScheduleSystem::removeEntry(const std::string& entity_id, int index) {
    auto* sched = getComponentFor(entity_id);
    if (!sched) return false;
    if (index < 0 || index >= static_cast<int>(sched->schedule.size())) return false;
    sched->schedule.erase(sched->schedule.begin() + index);
    return true;
}

bool NPCScheduleSystem::clearSchedule(const std::string& entity_id) {
    auto* sched = getComponentFor(entity_id);
    if (!sched) return false;
    sched->schedule.clear();
    sched->current_activity = Activity::Idle;
    return true;
}

bool NPCScheduleSystem::setCurrentHour(const std::string& entity_id, float hour) {
    auto* sched = getComponentFor(entity_id);
    if (!sched) return false;
    sched->current_hour = std::fmod(std::max(0.0f, hour), 24.0f);
    sched->elapsed_day_time = (sched->current_hour / 24.0f) * sched->day_length;
    return true;
}

bool NPCScheduleSystem::setDayLength(const std::string& entity_id, float length_seconds) {
    auto* sched = getComponentFor(entity_id);
    if (!sched) return false;
    if (length_seconds <= 0.0f) return false;
    sched->day_length = length_seconds;
    return true;
}

int NPCScheduleSystem::getEntryCount(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    return sched ? static_cast<int>(sched->schedule.size()) : 0;
}

int NPCScheduleSystem::getCurrentActivity(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    return sched ? static_cast<int>(sched->current_activity) : 0;
}

float NPCScheduleSystem::getCurrentHour(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    return sched ? sched->current_hour : 0.0f;
}

int NPCScheduleSystem::getTransitions(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    return sched ? sched->transitions : 0;
}

int NPCScheduleSystem::getDaysCompleted(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    return sched ? sched->days_completed : 0;
}

float NPCScheduleSystem::getAdherenceScore(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    return sched ? sched->adherence_score : 0.0f;
}

std::string NPCScheduleSystem::getCurrentLocation(const std::string& entity_id) const {
    auto* sched = getComponentFor(entity_id);
    if (!sched) return "";
    const Entry* active = findActiveEntry(sched, sched->current_hour);
    return active ? active->location : "";
}

} // namespace systems
} // namespace atlas
