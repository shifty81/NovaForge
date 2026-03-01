#include "systems/fps_interaction_system.h"
#include "ecs/world.h"
#include <cmath>
#include <limits>

namespace atlas {
namespace systems {

FPSInteractionSystem::FPSInteractionSystem(ecs::World* world)
    : System(world) {
}

void FPSInteractionSystem::update(float /*delta_time*/) {
    // Interaction is event-driven (on demand), no per-tick work required.
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

bool FPSInteractionSystem::createInteractable(
        const std::string& interactable_id,
        const std::string& interior_id,
        const std::string& linked_entity_id,
        components::FPSInteractable::InteractionType type,
        float x, float y, float z,
        float range,
        const std::string& display_name) {

    if (world_->getEntity(interactable_id)) return false;

    auto* entity = world_->createEntity(interactable_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::FPSInteractable>();
    comp->interactable_id   = interactable_id;
    comp->interior_id       = interior_id;
    comp->linked_entity_id  = linked_entity_id;
    comp->interaction_type  = static_cast<int>(type);
    comp->pos_x             = x;
    comp->pos_y             = y;
    comp->pos_z             = z;
    comp->interact_range    = range;
    comp->display_name      = display_name.empty()
                                ? typeName(static_cast<int>(type))
                                : display_name;
    entity->addComponent(std::move(comp));
    return true;
}

bool FPSInteractionSystem::setEnabled(const std::string& interactable_id,
                                       bool enabled) {
    auto* entity = world_->getEntity(interactable_id);
    if (!entity) return false;
    auto* ia = entity->getComponent<components::FPSInteractable>();
    if (!ia) return false;
    ia->is_enabled = enabled;
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

std::string FPSInteractionSystem::findNearestInteractable(
        const std::string& player_id) const {

    std::string char_eid = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* char_entity = world_->getEntity(char_eid);
    if (!char_entity) return "";
    auto* cs = char_entity->getComponent<components::FPSCharacterState>();
    if (!cs) return "";

    float best_dist = std::numeric_limits<float>::max();
    std::string best_id;

    for (auto* ent : world_->getEntities<components::FPSInteractable>()) {
        auto* ia = ent->getComponent<components::FPSInteractable>();
        if (!ia || !ia->is_enabled) continue;
        // Must be in the same interior
        if (ia->interior_id != cs->interior_id) continue;

        float dx = ia->pos_x - cs->pos_x;
        float dy = ia->pos_y - cs->pos_y;
        float dz = ia->pos_z - cs->pos_z;
        float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

        if (dist <= ia->interact_range && dist < best_dist) {
            best_dist = dist;
            best_id = ia->interactable_id;
        }
    }

    return best_id;
}

float FPSInteractionSystem::getDistanceTo(
        const std::string& player_id,
        const std::string& interactable_id) const {

    std::string char_eid = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* char_entity = world_->getEntity(char_eid);
    if (!char_entity) return -1.0f;
    auto* cs = char_entity->getComponent<components::FPSCharacterState>();
    if (!cs) return -1.0f;

    auto* ia_entity = world_->getEntity(interactable_id);
    if (!ia_entity) return -1.0f;
    auto* ia = ia_entity->getComponent<components::FPSInteractable>();
    if (!ia) return -1.0f;

    float dx = ia->pos_x - cs->pos_x;
    float dy = ia->pos_y - cs->pos_y;
    float dz = ia->pos_z - cs->pos_z;
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

bool FPSInteractionSystem::isInRange(
        const std::string& player_id,
        const std::string& interactable_id) const {

    auto* ia_entity = world_->getEntity(interactable_id);
    if (!ia_entity) return false;
    auto* ia = ia_entity->getComponent<components::FPSInteractable>();
    if (!ia) return false;

    float dist = getDistanceTo(player_id, interactable_id);
    return dist >= 0.0f && dist <= ia->interact_range;
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

bool FPSInteractionSystem::interact(
        const std::string& player_id,
        const std::string& interactable_id,
        const std::string& player_access) {

    // Resolve target
    std::string target_id = interactable_id;
    if (target_id.empty()) {
        target_id = findNearestInteractable(player_id);
    }
    if (target_id.empty()) return false;

    // Verify range
    if (!isInRange(player_id, target_id)) return false;

    auto* ia_entity = world_->getEntity(target_id);
    if (!ia_entity) return false;
    auto* ia = ia_entity->getComponent<components::FPSInteractable>();
    if (!ia || !ia->is_enabled) return false;

    // Access check
    if (!ia->required_access.empty() && player_access != ia->required_access) {
        return false;
    }

    using IT = components::FPSInteractable::InteractionType;
    int type = ia->interaction_type;

    if (type == static_cast<int>(IT::Door)) {
        // Toggle door via InteriorDoorSystem interface — find door entity
        auto* door_entity = world_->getEntity(ia->linked_entity_id);
        if (!door_entity) return false;
        auto* door = door_entity->getComponent<components::InteriorDoor>();
        if (!door) return false;

        using DS = components::InteriorDoor::DoorState;
        int ds = door->door_state;
        if (ds == static_cast<int>(DS::Closed)) {
            // Attempt to open (handles airlock/security checks inline)
            if (door->is_locked) return false;
            if (door->door_type == static_cast<int>(components::InteriorDoor::DoorType::Security)) {
                if (!door->required_access.empty() && player_access != door->required_access)
                    return false;
            }
            if (door->door_type == static_cast<int>(components::InteriorDoor::DoorType::Airlock)) {
                float diff = std::fabs(door->pressure_a - door->pressure_b);
                if (diff > door->pressure_threshold) return false;
            }
            door->door_state = static_cast<int>(DS::Opening);
            return true;
        } else if (ds == static_cast<int>(DS::Open)) {
            door->door_state = static_cast<int>(DS::Closing);
            return true;
        }
        return false;
    }

    if (type == static_cast<int>(IT::Airlock)) {
        // Toggle airlock — begin EVA or reentry
        auto* al_entity = world_->getEntity(ia->linked_entity_id);
        if (!al_entity) return false;
        auto* al = al_entity->getComponent<components::EVAAirlockState>();
        if (!al) return false;

        using P = components::EVAAirlockState::Phase;
        if (al->phase == static_cast<int>(P::Idle)) {
            // Check suit oxygen from survival needs
            float suit_oxy = 100.0f;
            std::string char_eid = std::string(FPS_CHAR_PREFIX) + player_id;
            auto* char_entity = world_->getEntity(char_eid);
            if (char_entity) {
                auto* needs = char_entity->getComponent<components::SurvivalNeeds>();
                if (needs) suit_oxy = needs->oxygen;
            }
            if (suit_oxy < al->min_suit_oxygen) return false;
            al->suit_check_passed = true;
            al->player_id = player_id;
            al->inner_door_open = true;
            al->phase = static_cast<int>(P::EnterChamber);
            al->phase_progress = 0.0f;
            al->abort_requested = false;
            return true;
        } else if (al->phase == static_cast<int>(P::EVAActive) &&
                   al->player_id == player_id) {
            al->phase = static_cast<int>(P::OuterSeal);
            al->phase_progress = 0.0f;
            return true;
        }
        return false;
    }

    if (type == static_cast<int>(IT::MedicalBay)) {
        // Heal the player via SurvivalSystem needs
        std::string char_eid = std::string(FPS_CHAR_PREFIX) + player_id;
        auto* char_entity = world_->getEntity(char_eid);
        if (!char_entity) return false;
        auto* needs = char_entity->getComponent<components::SurvivalNeeds>();
        if (!needs) return false;
        needs->oxygen = 100.0f;
        needs->hunger = std::max(0.0f, needs->hunger - 50.0f);
        needs->fatigue = std::max(0.0f, needs->fatigue - 50.0f);
        return true;
    }

    if (type == static_cast<int>(IT::Terminal) ||
        type == static_cast<int>(IT::LootContainer) ||
        type == static_cast<int>(IT::Fabricator)) {
        // These are valid interaction types — return true to signal UI should open.
        // Actual logic (loot generation, crafting) lives in other systems.
        return true;
    }

    return false;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string FPSInteractionSystem::typeName(int type) {
    using IT = components::FPSInteractable::InteractionType;
    switch (static_cast<IT>(type)) {
        case IT::Door:          return "Door";
        case IT::Airlock:       return "Airlock";
        case IT::Terminal:      return "Terminal";
        case IT::LootContainer: return "LootContainer";
        case IT::Fabricator:    return "Fabricator";
        case IT::MedicalBay:    return "MedicalBay";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
