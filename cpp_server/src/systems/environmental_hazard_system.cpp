#include "systems/environmental_hazard_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

EnvironmentalHazardSystem::EnvironmentalHazardSystem(ecs::World* world)
    : System(world) {
}

void EnvironmentalHazardSystem::update(float delta_time) {
    for (auto* entity : world_->getEntities<components::EnvironmentalHazard>()) {
        auto* hazard = entity->getComponent<components::EnvironmentalHazard>();
        if (!hazard || !hazard->is_active) continue;

        // Advance spread timer
        if (!hazard->is_spreading) {
            hazard->spread_timer += delta_time;
            if (hazard->spread_timer >= hazard->spread_interval) {
                hazard->is_spreading = true;
            }
        }

        // Advance repair
        if (hazard->is_being_repaired) {
            hazard->repair_progress += hazard->repair_rate * delta_time;
            if (hazard->repair_progress >= 1.0f) {
                hazard->repair_progress = 1.0f;
                hazard->is_active = false;
                hazard->is_being_repaired = false;
                hazard->is_spreading = false;
            }
        }

        // Apply damage to FPS characters in the same room.
        // Characters are matched by interior_id AND current_room_id so
        // that only those physically present in the hazard's room take
        // damage.  Characters without a current_room_id are unaffected.
        for (auto* charEntity : world_->getEntities<components::FPSCharacterState>()) {
            auto* cs = charEntity->getComponent<components::FPSCharacterState>();
            if (!cs || cs->interior_id != hazard->interior_id) continue;
            if (cs->current_room_id.empty() || cs->current_room_id != hazard->room_id) continue;

            auto* health = charEntity->getComponent<components::FPSHealth>();
            if (health && health->is_alive) {
                float dmg = hazard->damage_per_second * delta_time;
                if (health->shield > 0.0f) {
                    float absorbed = std::min(health->shield, dmg);
                    health->shield -= absorbed;
                    dmg -= absorbed;
                }
                if (dmg > 0.0f) {
                    health->health -= dmg;
                    if (health->health <= 0.0f) {
                        health->health = 0.0f;
                        health->is_alive = false;
                    }
                }
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

bool EnvironmentalHazardSystem::createHazard(
        const std::string& hazard_id,
        const std::string& room_id,
        const std::string& interior_id,
        components::EnvironmentalHazard::HazardType type,
        components::EnvironmentalHazard::Severity severity) {

    if (world_->getEntity(hazard_id)) return false;

    auto* entity = world_->createEntity(hazard_id);
    if (!entity) return false;

    int sev = static_cast<int>(severity);

    auto comp = std::make_unique<components::EnvironmentalHazard>();
    comp->hazard_id        = hazard_id;
    comp->room_id          = room_id;
    comp->interior_id      = interior_id;
    comp->hazard_type      = static_cast<int>(type);
    comp->severity         = sev;
    comp->damage_per_second = getDPSForSeverity(sev);
    comp->spread_interval  = getSpreadIntervalForSeverity(sev);

    entity->addComponent(std::move(comp));
    return true;
}

bool EnvironmentalHazardSystem::removeHazard(const std::string& hazard_id) {
    auto* entity = world_->getEntity(hazard_id);
    if (!entity) return false;
    auto* hazard = entity->getComponent<components::EnvironmentalHazard>();
    if (!hazard) return false;
    hazard->is_active = false;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::vector<std::string> EnvironmentalHazardSystem::getHazardsInRoom(
        const std::string& room_id) const {
    std::vector<std::string> result;
    for (auto* entity : world_->getEntities<components::EnvironmentalHazard>()) {
        auto* h = entity->getComponent<components::EnvironmentalHazard>();
        if (h && h->is_active && h->room_id == room_id)
            result.push_back(h->hazard_id);
    }
    return result;
}

std::vector<std::string> EnvironmentalHazardSystem::getActiveHazards(
        const std::string& interior_id) const {
    std::vector<std::string> result;
    for (auto* entity : world_->getEntities<components::EnvironmentalHazard>()) {
        auto* h = entity->getComponent<components::EnvironmentalHazard>();
        if (h && h->is_active && h->interior_id == interior_id)
            result.push_back(h->hazard_id);
    }
    return result;
}

bool EnvironmentalHazardSystem::isRoomSafe(const std::string& room_id) const {
    return getHazardsInRoom(room_id).empty();
}

float EnvironmentalHazardSystem::getRoomDPS(const std::string& room_id) const {
    float total = 0.0f;
    for (auto* entity : world_->getEntities<components::EnvironmentalHazard>()) {
        auto* h = entity->getComponent<components::EnvironmentalHazard>();
        if (h && h->is_active && h->room_id == room_id)
            total += h->damage_per_second;
    }
    return total;
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

bool EnvironmentalHazardSystem::startRepair(const std::string& hazard_id) {
    auto* entity = world_->getEntity(hazard_id);
    if (!entity) return false;
    auto* h = entity->getComponent<components::EnvironmentalHazard>();
    if (!h || !h->is_active) return false;
    h->is_being_repaired = true;
    return true;
}

bool EnvironmentalHazardSystem::stopRepair(const std::string& hazard_id) {
    auto* entity = world_->getEntity(hazard_id);
    if (!entity) return false;
    auto* h = entity->getComponent<components::EnvironmentalHazard>();
    if (!h) return false;
    h->is_being_repaired = false;
    return true;
}

float EnvironmentalHazardSystem::getRepairProgress(
        const std::string& hazard_id) const {
    auto* entity = world_->getEntity(hazard_id);
    if (!entity) return 0.0f;
    auto* h = entity->getComponent<components::EnvironmentalHazard>();
    if (!h) return 0.0f;
    return h->repair_progress;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string EnvironmentalHazardSystem::hazardTypeName(int type) {
    return components::EnvironmentalHazard::hazardTypeName(type);
}

std::string EnvironmentalHazardSystem::severityName(int sev) {
    return components::EnvironmentalHazard::severityName(sev);
}

float EnvironmentalHazardSystem::getDPSForSeverity(int severity) const {
    using S = components::EnvironmentalHazard::Severity;
    switch (static_cast<S>(severity)) {
        case S::Minor:        return 2.0f;
        case S::Moderate:     return 5.0f;
        case S::Critical:     return 12.0f;
        case S::Catastrophic: return 25.0f;
        default:              return 5.0f;
    }
}

float EnvironmentalHazardSystem::getSpreadIntervalForSeverity(int severity) const {
    using S = components::EnvironmentalHazard::Severity;
    switch (static_cast<S>(severity)) {
        case S::Minor:        return 60.0f;
        case S::Moderate:     return 30.0f;
        case S::Critical:     return 15.0f;
        case S::Catastrophic: return 5.0f;
        default:              return 30.0f;
    }
}

} // namespace systems
} // namespace atlas
