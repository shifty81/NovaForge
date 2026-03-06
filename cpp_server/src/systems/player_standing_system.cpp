#include "systems/player_standing_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

PlayerStandingSystem::PlayerStandingSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void PlayerStandingSystem::updateComponent(ecs::Entity& /*entity*/,
    components::PlayerStanding& ps, float delta_time) {
    if (!ps.active) return;
    ps.elapsed += delta_time;
}

int PlayerStandingSystem::computeRank(double standing) {
    if (standing >= 8.0) return 5;
    if (standing >= 5.0) return 4;
    if (standing >= 2.0) return 3;
    if (standing >= 0.5) return 2;
    if (standing >= 0.1) return 1;
    if (standing > -0.1) return 0;
    if (standing > -0.5) return -1;
    if (standing > -2.0) return -2;
    if (standing > -5.0) return -3;
    if (standing > -8.0) return -4;
    return -5;
}

bool PlayerStandingSystem::addFaction(const std::string& entity_id,
    const std::string& faction_id, const std::string& faction_name) {
    auto* ps = getComponentFor(entity_id);
    if (!ps) return false;

    for (const auto& f : ps->factions) {
        if (f.faction_id == faction_id) return false;  // Already exists
    }

    components::PlayerStanding::FactionStanding fs;
    fs.faction_id = faction_id;
    fs.faction_name = faction_name;
    ps->factions.push_back(fs);
    return true;
}

bool PlayerStandingSystem::modifyStanding(const std::string& entity_id,
    const std::string& faction_id, double delta) {
    auto* ps = getComponentFor(entity_id);
    if (!ps) return false;

    for (auto& f : ps->factions) {
        if (f.faction_id == faction_id) {
            int old_rank = computeRank(f.standing);
            f.standing = std::max(-10.0, std::min(10.0, f.standing + delta));
            int new_rank = computeRank(f.standing);
            f.rank = new_rank;

            if (old_rank != new_rank) {
                if (static_cast<int>(ps->notifications.size()) >= ps->max_notifications) {
                    ps->notifications.erase(ps->notifications.begin());
                }
                components::PlayerStanding::StandingNotification notif;
                notif.faction_id = faction_id;
                notif.old_rank = old_rank;
                notif.new_rank = new_rank;
                notif.timestamp = ps->elapsed;
                ps->notifications.push_back(notif);
            }
            return true;
        }
    }
    return false;
}

double PlayerStandingSystem::getStanding(const std::string& entity_id,
    const std::string& faction_id) const {
    auto* ps = getComponentFor(entity_id);
    if (!ps) return 0.0;
    for (const auto& f : ps->factions) {
        if (f.faction_id == faction_id) return f.standing;
    }
    return 0.0;
}

int PlayerStandingSystem::getRank(const std::string& entity_id,
    const std::string& faction_id) const {
    auto* ps = getComponentFor(entity_id);
    if (!ps) return 0;
    for (const auto& f : ps->factions) {
        if (f.faction_id == faction_id) return f.rank;
    }
    return 0;
}

int PlayerStandingSystem::getFactionCount(const std::string& entity_id) const {
    auto* ps = getComponentFor(entity_id);
    return ps ? static_cast<int>(ps->factions.size()) : 0;
}

int PlayerStandingSystem::getNotificationCount(const std::string& entity_id) const {
    auto* ps = getComponentFor(entity_id);
    return ps ? static_cast<int>(ps->notifications.size()) : 0;
}

bool PlayerStandingSystem::hasFaction(const std::string& entity_id,
    const std::string& faction_id) const {
    auto* ps = getComponentFor(entity_id);
    if (!ps) return false;
    for (const auto& f : ps->factions) {
        if (f.faction_id == faction_id) return true;
    }
    return false;
}

std::string PlayerStandingSystem::getRankName(int rank) const {
    switch (rank) {
        case 5: return "Hero";
        case 4: return "Ally";
        case 3: return "Friend";
        case 2: return "Associate";
        case 1: return "Acquaintance";
        case 0: return "Neutral";
        case -1: return "Suspect";
        case -2: return "Unfriendly";
        case -3: return "Hostile";
        case -4: return "Enemy";
        case -5: return "Nemesis";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
