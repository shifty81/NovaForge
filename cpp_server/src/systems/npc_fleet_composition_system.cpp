#include "systems/npc_fleet_composition_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

namespace {

using NFC = components::NpcFleetComposition;

const char* difficultyToString(NFC::DifficultyTier d) {
    switch (d) {
        case NFC::DifficultyTier::Easy:   return "Easy";
        case NFC::DifficultyTier::Medium: return "Medium";
        case NFC::DifficultyTier::Hard:   return "Hard";
        case NFC::DifficultyTier::Elite:  return "Elite";
    }
    return "Unknown";
}

NFC::DifficultyTier stringToDifficulty(const std::string& s) {
    if (s == "Medium") return NFC::DifficultyTier::Medium;
    if (s == "Hard")   return NFC::DifficultyTier::Hard;
    if (s == "Elite")  return NFC::DifficultyTier::Elite;
    return NFC::DifficultyTier::Easy;
}

NFC::ShipRole stringToRole(const std::string& s) {
    if (s == "Tank")      return NFC::ShipRole::Tank;
    if (s == "Support")   return NFC::ShipRole::Support;
    if (s == "Commander") return NFC::ShipRole::Commander;
    if (s == "Scout")     return NFC::ShipRole::Scout;
    return NFC::ShipRole::DPS;
}

float difficultyMultiplier(NFC::DifficultyTier d) {
    switch (d) {
        case NFC::DifficultyTier::Easy:   return 1.0f;
        case NFC::DifficultyTier::Medium: return 1.5f;
        case NFC::DifficultyTier::Hard:   return 2.0f;
        case NFC::DifficultyTier::Elite:  return 3.0f;
    }
    return 1.0f;
}

void recalcThreat(NFC& comp) {
    float base = 0.0f;
    for (const auto& ship : comp.ship_roster) {
        base += ship.threat_value;
    }
    comp.difficulty_scale = difficultyMultiplier(comp.difficulty);
    comp.total_threat = base * comp.difficulty_scale;
}

} // anonymous namespace

NpcFleetCompositionSystem::NpcFleetCompositionSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void NpcFleetCompositionSystem::updateComponent(ecs::Entity& entity,
    components::NpcFleetComposition& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Tick spawn cooldown
    if (comp.cooldown_timer > 0.0f) {
        comp.cooldown_timer = std::max(0.0f, comp.cooldown_timer - delta_time);
    }

    recalcThreat(comp);
}

bool NpcFleetCompositionSystem::initialize(const std::string& entity_id,
    const std::string& template_id, const std::string& template_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::NpcFleetComposition>();
    comp->template_id = template_id;
    comp->template_name = template_name;
    entity->addComponent(std::move(comp));
    return true;
}

bool NpcFleetCompositionSystem::addShip(const std::string& entity_id,
    const std::string& ship_type, const std::string& role,
    float threat_value, bool is_commander) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (static_cast<int>(comp->ship_roster.size()) >= comp->max_ships) return false;

    NFC::ShipEntry entry;
    entry.ship_type = ship_type;
    entry.role = stringToRole(role);
    entry.threat_value = threat_value;
    entry.is_commander = is_commander;
    comp->ship_roster.push_back(entry);
    recalcThreat(*comp);
    return true;
}

bool NpcFleetCompositionSystem::setSecurityLevel(const std::string& entity_id,
    float security_level) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->security_level = security_level;
    return true;
}

bool NpcFleetCompositionSystem::setDifficulty(const std::string& entity_id,
    const std::string& difficulty) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->difficulty = stringToDifficulty(difficulty);
    recalcThreat(*comp);
    return true;
}

bool NpcFleetCompositionSystem::setSpawnCooldown(const std::string& entity_id,
    float cooldown) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->spawn_cooldown = cooldown;
    return true;
}

bool NpcFleetCompositionSystem::requestSpawn(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (comp->cooldown_timer > 0.0f) return false;
    comp->fleets_spawned++;
    comp->cooldown_timer = comp->spawn_cooldown;
    return true;
}

bool NpcFleetCompositionSystem::recordDestroyed(const std::string& entity_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->fleets_destroyed++;
    return true;
}

int NpcFleetCompositionSystem::getShipCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->ship_roster.size()) : 0;
}

float NpcFleetCompositionSystem::getTotalThreat(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_threat : 0.0f;
}

std::string NpcFleetCompositionSystem::getDifficulty(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return "Unknown";
    return difficultyToString(comp->difficulty);
}

int NpcFleetCompositionSystem::getFleetsSpawned(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleets_spawned : 0;
}

int NpcFleetCompositionSystem::getFleetsDestroyed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->fleets_destroyed : 0;
}

float NpcFleetCompositionSystem::getCooldownRemaining(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->cooldown_timer : 0.0f;
}

bool NpcFleetCompositionSystem::isReady(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? (comp->cooldown_timer <= 0.0f) : false;
}

} // namespace systems
} // namespace atlas
