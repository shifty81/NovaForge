#include "systems/damage_notification_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

DamageNotificationSystem::DamageNotificationSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DamageNotificationSystem::updateComponent(ecs::Entity& /*entity*/,
    components::DamageNotification& notif, float delta_time) {
    if (!notif.active) return;
    notif.elapsed += delta_time;

    // Remove expired incoming entries
    float cutoff = notif.elapsed - notif.entry_lifetime;
    notif.incoming.erase(
        std::remove_if(notif.incoming.begin(), notif.incoming.end(),
            [cutoff](const components::DamageNotification::DamageEntry& e) {
                return e.timestamp < cutoff;
            }),
        notif.incoming.end());

    // Remove expired outgoing entries
    notif.outgoing.erase(
        std::remove_if(notif.outgoing.begin(), notif.outgoing.end(),
            [cutoff](const components::DamageNotification::DamageEntry& e) {
                return e.timestamp < cutoff;
            }),
        notif.outgoing.end());
}

bool DamageNotificationSystem::recordIncoming(const std::string& entity_id,
    const std::string& attacker_id, float amount, int damage_type,
    const std::string& weapon, bool is_crit) {
    if (amount <= 0.0f) return false;
    auto* notif = getComponentFor(entity_id);
    if (!notif) return false;

    if (static_cast<int>(notif->incoming.size()) >= notif->max_entries) {
        notif->incoming.erase(notif->incoming.begin());
    }

    components::DamageNotification::DamageEntry entry;
    entry.source_id = attacker_id;
    entry.amount = amount;
    entry.damage_type = damage_type;
    entry.weapon_name = weapon;
    entry.is_critical = is_crit;
    entry.timestamp = notif->elapsed;
    notif->incoming.push_back(entry);

    notif->total_damage_taken += amount;
    notif->hits_taken++;
    if (is_crit) notif->crits_taken++;
    return true;
}

bool DamageNotificationSystem::recordOutgoing(const std::string& entity_id,
    const std::string& target_id, float amount, int damage_type,
    const std::string& weapon, bool is_crit) {
    if (amount <= 0.0f) return false;
    auto* notif = getComponentFor(entity_id);
    if (!notif) return false;

    if (static_cast<int>(notif->outgoing.size()) >= notif->max_entries) {
        notif->outgoing.erase(notif->outgoing.begin());
    }

    components::DamageNotification::DamageEntry entry;
    entry.source_id = target_id;
    entry.amount = amount;
    entry.damage_type = damage_type;
    entry.weapon_name = weapon;
    entry.is_critical = is_crit;
    entry.timestamp = notif->elapsed;
    notif->outgoing.push_back(entry);

    notif->total_damage_dealt += amount;
    notif->hits_dealt++;
    if (is_crit) notif->crits_dealt++;
    return true;
}

int DamageNotificationSystem::getIncomingCount(const std::string& entity_id) const {
    auto* notif = getComponentFor(entity_id);
    return notif ? static_cast<int>(notif->incoming.size()) : 0;
}

int DamageNotificationSystem::getOutgoingCount(const std::string& entity_id) const {
    auto* notif = getComponentFor(entity_id);
    return notif ? static_cast<int>(notif->outgoing.size()) : 0;
}

float DamageNotificationSystem::getTotalDamageTaken(const std::string& entity_id) const {
    auto* notif = getComponentFor(entity_id);
    return notif ? notif->total_damage_taken : 0.0f;
}

float DamageNotificationSystem::getTotalDamageDealt(const std::string& entity_id) const {
    auto* notif = getComponentFor(entity_id);
    return notif ? notif->total_damage_dealt : 0.0f;
}

int DamageNotificationSystem::getCritsTaken(const std::string& entity_id) const {
    auto* notif = getComponentFor(entity_id);
    return notif ? notif->crits_taken : 0;
}

int DamageNotificationSystem::getCritsDealt(const std::string& entity_id) const {
    auto* notif = getComponentFor(entity_id);
    return notif ? notif->crits_dealt : 0;
}

float DamageNotificationSystem::getRecentDPS(const std::string& entity_id, float window) const {
    if (window <= 0.0f) return 0.0f;
    auto* notif = getComponentFor(entity_id);
    if (!notif) return 0.0f;

    float cutoff = notif->elapsed - window;
    float total = 0.0f;
    for (const auto& e : notif->outgoing) {
        if (e.timestamp >= cutoff) {
            total += e.amount;
        }
    }
    return total / window;
}

bool DamageNotificationSystem::clearNotifications(const std::string& entity_id) {
    auto* notif = getComponentFor(entity_id);
    if (!notif) return false;
    notif->incoming.clear();
    notif->outgoing.clear();
    return true;
}

} // namespace systems
} // namespace atlas
