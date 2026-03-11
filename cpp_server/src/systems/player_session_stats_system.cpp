#include "systems/player_session_stats_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"

namespace atlas {
namespace systems {

PlayerSessionStatsSystem::PlayerSessionStatsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

// ---------------------------------------------------------------------------
// Tick
// ---------------------------------------------------------------------------

void PlayerSessionStatsSystem::updateComponent(
        ecs::Entity& /*entity*/,
        components::PlayerSessionStats& comp,
        float delta_time) {
    if (!comp.active) return;
    comp.elapsed_time += delta_time;
}

// ---------------------------------------------------------------------------
// Lifecycle
// ---------------------------------------------------------------------------

bool PlayerSessionStatsSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PlayerSessionStats>();
    entity->addComponent(std::move(comp));
    return true;
}

bool PlayerSessionStatsSystem::startSession(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    // Reset per-session counters
    comp->kills             = 0;
    comp->losses            = 0;
    comp->damage_dealt      = 0.0f;
    comp->damage_received   = 0.0f;
    comp->assists           = 0;
    comp->isk_earned        = 0.0f;
    comp->isk_spent         = 0.0f;
    comp->trades_completed  = 0;
    comp->items_looted      = 0;
    comp->distance_traveled = 0.0f;
    comp->jumps_made        = 0;
    comp->warps_made        = 0;
    comp->elapsed_time      = 0.0f;
    comp->active            = true;
    comp->total_sessions++;
    return true;
}

bool PlayerSessionStatsSystem::endSession(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->active = false;
    return true;
}

bool PlayerSessionStatsSystem::resetSession(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->kills             = 0;
    comp->losses            = 0;
    comp->damage_dealt      = 0.0f;
    comp->damage_received   = 0.0f;
    comp->assists           = 0;
    comp->isk_earned        = 0.0f;
    comp->isk_spent         = 0.0f;
    comp->trades_completed  = 0;
    comp->items_looted      = 0;
    comp->distance_traveled = 0.0f;
    comp->jumps_made        = 0;
    comp->warps_made        = 0;
    comp->elapsed_time      = 0.0f;
    comp->active            = true;
    return true;
}

// ---------------------------------------------------------------------------
// Combat recording
// ---------------------------------------------------------------------------

bool PlayerSessionStatsSystem::recordKill(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->kills++;
    return true;
}

bool PlayerSessionStatsSystem::recordLoss(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->losses++;
    return true;
}

bool PlayerSessionStatsSystem::recordDamageDealt(const std::string& entity_id,
                                                   float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount < 0.0f) return false;
    comp->damage_dealt += amount;
    return true;
}

bool PlayerSessionStatsSystem::recordDamageReceived(
        const std::string& entity_id, float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount < 0.0f) return false;
    comp->damage_received += amount;
    return true;
}

bool PlayerSessionStatsSystem::recordAssist(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->assists++;
    return true;
}

// ---------------------------------------------------------------------------
// Economy recording
// ---------------------------------------------------------------------------

bool PlayerSessionStatsSystem::recordIskEarned(const std::string& entity_id,
                                                 float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount < 0.0f) return false;
    comp->isk_earned += amount;
    return true;
}

bool PlayerSessionStatsSystem::recordIskSpent(const std::string& entity_id,
                                               float amount) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (amount < 0.0f) return false;
    comp->isk_spent += amount;
    return true;
}

bool PlayerSessionStatsSystem::recordTradeCompleted(
        const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->trades_completed++;
    return true;
}

bool PlayerSessionStatsSystem::recordItemLooted(const std::string& entity_id,
                                                  int count) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (count <= 0) return false;
    comp->items_looted += count;
    return true;
}

// ---------------------------------------------------------------------------
// Travel recording
// ---------------------------------------------------------------------------

bool PlayerSessionStatsSystem::recordDistanceTraveled(
        const std::string& entity_id, float au) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (au < 0.0f) return false;
    comp->distance_traveled += au;
    return true;
}

bool PlayerSessionStatsSystem::recordJump(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->jumps_made++;
    return true;
}

bool PlayerSessionStatsSystem::recordWarp(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->warps_made++;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int PlayerSessionStatsSystem::getKills(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->kills : 0;
}

int PlayerSessionStatsSystem::getLosses(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->losses : 0;
}

float PlayerSessionStatsSystem::getDamageDealt(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->damage_dealt : 0.0f;
}

float PlayerSessionStatsSystem::getDamageReceived(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->damage_received : 0.0f;
}

int PlayerSessionStatsSystem::getAssists(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->assists : 0;
}

float PlayerSessionStatsSystem::getIskEarned(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->isk_earned : 0.0f;
}

float PlayerSessionStatsSystem::getIskSpent(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->isk_spent : 0.0f;
}

float PlayerSessionStatsSystem::getNetISK(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->isk_earned - comp->isk_spent) : 0.0f;
}

int PlayerSessionStatsSystem::getTradesCompleted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->trades_completed : 0;
}

int PlayerSessionStatsSystem::getItemsLooted(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->items_looted : 0;
}

float PlayerSessionStatsSystem::getDistanceTraveled(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->distance_traveled : 0.0f;
}

int PlayerSessionStatsSystem::getJumpsMade(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->jumps_made : 0;
}

int PlayerSessionStatsSystem::getWarpsMade(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->warps_made : 0;
}

float PlayerSessionStatsSystem::getElapsedTime(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->elapsed_time : 0.0f;
}

float PlayerSessionStatsSystem::getKDRatio(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    if (comp->losses == 0) return static_cast<float>(comp->kills);
    return static_cast<float>(comp->kills) / static_cast<float>(comp->losses);
}

int PlayerSessionStatsSystem::getTotalSessions(
        const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_sessions : 0;
}

bool PlayerSessionStatsSystem::isActive(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->active : false;
}

} // namespace systems
} // namespace atlas
