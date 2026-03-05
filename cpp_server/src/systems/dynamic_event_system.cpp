#include "systems/dynamic_event_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::DynamicEvent::EventEntry* findEvent(
    components::DynamicEvent* de, const std::string& event_id) {
    for (auto& ev : de->events) {
        if (ev.event_id == event_id) return &ev;
    }
    return nullptr;
}
} // anonymous namespace

DynamicEventSystem::DynamicEventSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void DynamicEventSystem::updateComponent(ecs::Entity& entity, components::DynamicEvent& de, float delta_time) {
    if (!de.active) return;

    for (auto& ev : de.events) {
        if (ev.state == "Pending") {
            ev.start_delay -= delta_time;
            if (ev.start_delay <= 0.0f) {
                ev.state = "Active";
                ev.start_delay = 0.0f;
            }
        } else if (ev.state == "Active") {
            ev.elapsed_time += delta_time;
            ev.reward_pool += ev.intensity * delta_time * 100.0f;
            float progress = ev.elapsed_time / ev.duration;
            if (progress >= 1.0f) {
                ev.state = "Completed";
                ev.elapsed_time = ev.duration;
                de.total_completed++;
            } else if (progress >= 0.8f) {
                ev.state = "Concluding";
            }
        } else if (ev.state == "Concluding") {
            ev.elapsed_time += delta_time;
            ev.reward_pool += ev.intensity * delta_time * 100.0f;
            float progress = ev.elapsed_time / ev.duration;
            if (progress >= 1.0f) {
                ev.state = "Completed";
                ev.elapsed_time = ev.duration;
                de.total_completed++;
            }
        }
    }
}

bool DynamicEventSystem::createEventManager(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DynamicEvent>();
    entity->addComponent(std::move(comp));
    return true;
}

bool DynamicEventSystem::scheduleEvent(
    const std::string& entity_id, const std::string& event_id,
    const std::string& type, float duration, float intensity) {
    auto* de = getComponentFor(entity_id);
    if (!de) return false;

    // Count active/pending events
    int active_count = 0;
    for (const auto& ev : de->events) {
        if (ev.state != "Completed") active_count++;
    }
    if (active_count >= de->max_concurrent_events) return false;

    // Check for duplicate event_id
    if (findEvent(de, event_id)) return false;

    components::DynamicEvent::EventEntry entry;
    entry.event_id = event_id;
    entry.type = type;
    entry.duration = duration;
    entry.intensity = std::max(0.0f, std::min(intensity, 1.0f));
    entry.state = "Pending";
    entry.start_delay = 5.0f;
    de->events.push_back(entry);
    return true;
}

bool DynamicEventSystem::startEvent(const std::string& entity_id,
                                    const std::string& event_id) {
    auto* de = getComponentFor(entity_id);
    if (!de) return false;
    auto* ev = findEvent(de, event_id);
    if (!ev || ev->state != "Pending") return false;
    ev->state = "Active";
    ev->start_delay = 0.0f;
    return true;
}

bool DynamicEventSystem::joinEvent(const std::string& entity_id,
                                   const std::string& event_id,
                                   const std::string& participant) {
    auto* de = getComponentFor(entity_id);
    if (!de) return false;
    auto* ev = findEvent(de, event_id);
    if (!ev || ev->state == "Completed") return false;
    // Check for duplicate participant
    for (const auto& p : ev->participants) {
        if (p == participant) return false;
    }
    ev->participants.push_back(participant);
    return true;
}

bool DynamicEventSystem::leaveEvent(const std::string& entity_id,
                                    const std::string& event_id,
                                    const std::string& participant) {
    auto* de = getComponentFor(entity_id);
    if (!de) return false;
    auto* ev = findEvent(de, event_id);
    if (!ev) return false;
    auto it = std::find(ev->participants.begin(), ev->participants.end(), participant);
    if (it == ev->participants.end()) return false;
    ev->participants.erase(it);
    return true;
}

std::string DynamicEventSystem::getEventState(const std::string& entity_id,
                                              const std::string& event_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return "";
    for (const auto& ev : de->events) {
        if (ev.event_id == event_id) return ev.state;
    }
    return "";
}

std::string DynamicEventSystem::getEventType(const std::string& entity_id,
                                             const std::string& event_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return "";
    for (const auto& ev : de->events) {
        if (ev.event_id == event_id) return ev.type;
    }
    return "";
}

int DynamicEventSystem::getParticipantCount(const std::string& entity_id,
                                            const std::string& event_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return 0;
    for (const auto& ev : de->events) {
        if (ev.event_id == event_id)
            return static_cast<int>(ev.participants.size());
    }
    return 0;
}

float DynamicEventSystem::getRewardPool(const std::string& entity_id,
                                        const std::string& event_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return 0.0f;
    for (const auto& ev : de->events) {
        if (ev.event_id == event_id) return ev.reward_pool;
    }
    return 0.0f;
}

float DynamicEventSystem::getIntensity(const std::string& entity_id,
                                       const std::string& event_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return 0.0f;
    for (const auto& ev : de->events) {
        if (ev.event_id == event_id) return ev.intensity;
    }
    return 0.0f;
}

int DynamicEventSystem::getActiveEventCount(const std::string& entity_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return 0;
    int count = 0;
    for (const auto& ev : de->events) {
        if (ev.state == "Active" || ev.state == "Concluding") count++;
    }
    return count;
}

int DynamicEventSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return 0;
    return de->total_completed;
}

float DynamicEventSystem::getElapsedTime(const std::string& entity_id,
                                         const std::string& event_id) const {
    auto* de = getComponentFor(entity_id);
    if (!de) return 0.0f;
    for (const auto& ev : de->events) {
        if (ev.event_id == event_id) return ev.elapsed_time;
    }
    return 0.0f;
}

bool DynamicEventSystem::cancelEvent(const std::string& entity_id,
                                     const std::string& event_id) {
    auto* de = getComponentFor(entity_id);
    if (!de) return false;
    auto* ev = findEvent(de, event_id);
    if (!ev || ev->state == "Completed") return false;
    ev->state = "Completed";
    return true;
}

} // namespace systems
} // namespace atlas
