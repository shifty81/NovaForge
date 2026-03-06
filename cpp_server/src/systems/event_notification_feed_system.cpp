#include "systems/event_notification_feed_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using ENF = components::EventNotificationFeed;
}

EventNotificationFeedSystem::EventNotificationFeedSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void EventNotificationFeedSystem::updateComponent(ecs::Entity& entity,
    components::EventNotificationFeed& state, float delta_time) {
    if (!state.active) return;
    state.elapsed_time += delta_time;

    // Expire notifications whose lifetime has elapsed
    for (auto it = state.notifications.begin(); it != state.notifications.end(); ) {
        it->age += delta_time;
        if (it->lifetime > 0.0f && it->age >= it->lifetime) {
            it = state.notifications.erase(it);
            state.total_expired++;
        } else {
            ++it;
        }
    }
}

bool EventNotificationFeedSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::EventNotificationFeed>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool EventNotificationFeedSystem::pushNotification(const std::string& entity_id,
    const std::string& event_id, const std::string& category,
    const std::string& message, int priority, float lifetime) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Duplicate prevention
    for (const auto& n : state->notifications) {
        if (n.event_id == event_id) return false;
    }
    // Evict lowest priority if at capacity
    if (static_cast<int>(state->notifications.size()) >= state->max_notifications) {
        auto lowest = std::min_element(state->notifications.begin(), state->notifications.end(),
            [](const ENF::Notification& a, const ENF::Notification& b) {
                return a.priority < b.priority;
            });
        if (lowest != state->notifications.end() && lowest->priority < priority) {
            state->notifications.erase(lowest);
        } else {
            return false;
        }
    }
    ENF::Notification notif;
    notif.event_id = event_id;
    notif.category = category;
    notif.message = message;
    notif.priority = priority;
    notif.lifetime = lifetime;
    state->notifications.push_back(notif);
    state->total_pushed++;
    return true;
}

int EventNotificationFeedSystem::getNotificationCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->notifications.size()) : 0;
}

bool EventNotificationFeedSystem::hasNotification(const std::string& entity_id,
    const std::string& event_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& n : state->notifications) {
        if (n.event_id == event_id) return true;
    }
    return false;
}

bool EventNotificationFeedSystem::markRead(const std::string& entity_id,
    const std::string& event_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (auto& n : state->notifications) {
        if (n.event_id == event_id) {
            if (n.read) return false;
            n.read = true;
            return true;
        }
    }
    return false;
}

bool EventNotificationFeedSystem::dismissNotification(const std::string& entity_id,
    const std::string& event_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->notifications.begin(), state->notifications.end(),
        [&](const ENF::Notification& n) { return n.event_id == event_id; });
    if (it == state->notifications.end()) return false;
    state->notifications.erase(it);
    return true;
}

int EventNotificationFeedSystem::getUnreadCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& n : state->notifications) {
        if (!n.read) count++;
    }
    return count;
}

int EventNotificationFeedSystem::getCountByCategory(const std::string& entity_id,
    const std::string& category) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    int count = 0;
    for (const auto& n : state->notifications) {
        if (n.category == category) count++;
    }
    return count;
}

std::string EventNotificationFeedSystem::getLatestMessage(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->notifications.empty()) return "";
    return state->notifications.back().message;
}

std::string EventNotificationFeedSystem::getLatestMessageInCategory(const std::string& entity_id,
    const std::string& category) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return "";
    for (auto it = state->notifications.rbegin(); it != state->notifications.rend(); ++it) {
        if (it->category == category) return it->message;
    }
    return "";
}

bool EventNotificationFeedSystem::clearAll(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    state->notifications.clear();
    return true;
}

bool EventNotificationFeedSystem::clearCategory(const std::string& entity_id,
    const std::string& category) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::remove_if(state->notifications.begin(), state->notifications.end(),
        [&](const ENF::Notification& n) { return n.category == category; });
    if (it == state->notifications.end()) return false;
    state->notifications.erase(it, state->notifications.end());
    return true;
}

int EventNotificationFeedSystem::getTotalPushed(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_pushed : 0;
}

int EventNotificationFeedSystem::getTotalExpired(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->total_expired : 0;
}

} // namespace systems
} // namespace atlas
