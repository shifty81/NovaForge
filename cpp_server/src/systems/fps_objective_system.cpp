#include "systems/fps_objective_system.h"
#include "ecs/world.h"

namespace atlas {
namespace systems {

FPSObjectiveSystem::FPSObjectiveSystem(ecs::World* world)
    : System(world) {
}

void FPSObjectiveSystem::update(float delta_time) {
    for (auto* entity : world_->getEntities<components::FPSObjective>()) {
        auto* obj = entity->getComponent<components::FPSObjective>();
        if (!obj) continue;

        using S = components::FPSObjective::ObjectiveState;
        if (obj->state != static_cast<int>(S::Active)) continue;

        // Advance elapsed time
        obj->elapsed_time += delta_time;

        // Check time limit
        if (obj->time_limit > 0.0f && obj->elapsed_time >= obj->time_limit) {
            obj->state = static_cast<int>(S::Failed);
            continue;
        }

        // Type-specific update
        using OT = components::FPSObjective::ObjectiveType;
        int otype = obj->objective_type;

        if (otype == static_cast<int>(OT::DefendPoint)) {
            obj->defend_elapsed += delta_time;
            if (obj->defend_duration > 0.0f) {
                obj->progress = std::min(1.0f, obj->defend_elapsed / obj->defend_duration);
                if (obj->defend_elapsed >= obj->defend_duration) {
                    obj->state = static_cast<int>(S::Completed);
                    obj->progress = 1.0f;
                }
            }
        } else if (otype == static_cast<int>(OT::EliminateHostiles)) {
            if (obj->hostiles_required > 0) {
                obj->progress = std::min(1.0f,
                    static_cast<float>(obj->hostiles_killed) /
                    static_cast<float>(obj->hostiles_required));
            }
        }
    }
}

// ---------------------------------------------------------------------------
// Setup
// ---------------------------------------------------------------------------

bool FPSObjectiveSystem::createObjective(
        const std::string& objective_id,
        const std::string& interior_id,
        const std::string& room_id,
        const std::string& player_id,
        components::FPSObjective::ObjectiveType type,
        const std::string& description,
        float time_limit) {

    if (world_->getEntity(objective_id)) return false;

    auto* entity = world_->createEntity(objective_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::FPSObjective>();
    comp->objective_id   = objective_id;
    comp->interior_id    = interior_id;
    comp->room_id        = room_id;
    comp->assigned_player = player_id;
    comp->objective_type = static_cast<int>(type);
    comp->state          = static_cast<int>(components::FPSObjective::ObjectiveState::Inactive);
    comp->description    = description.empty()
                            ? components::FPSObjective::objectiveTypeName(static_cast<int>(type))
                            : description;
    comp->time_limit     = time_limit;

    entity->addComponent(std::move(comp));
    return true;
}

bool FPSObjectiveSystem::activateObjective(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    if (obj->state != static_cast<int>(S::Inactive)) return false;
    obj->state = static_cast<int>(S::Active);
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

int FPSObjectiveSystem::getObjectiveState(const std::string& objective_id) const {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return -1;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return -1;
    return obj->state;
}

std::vector<std::string> FPSObjectiveSystem::getPlayerObjectives(
        const std::string& player_id) const {
    std::vector<std::string> result;
    for (auto* entity : world_->getEntities<components::FPSObjective>()) {
        auto* obj = entity->getComponent<components::FPSObjective>();
        if (obj && obj->assigned_player == player_id &&
            obj->state == static_cast<int>(components::FPSObjective::ObjectiveState::Active)) {
            result.push_back(obj->objective_id);
        }
    }
    return result;
}

float FPSObjectiveSystem::getProgress(const std::string& objective_id) const {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return 0.0f;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return 0.0f;
    return obj->progress;
}

bool FPSObjectiveSystem::isComplete(const std::string& objective_id) const {
    return getObjectiveState(objective_id) ==
           static_cast<int>(components::FPSObjective::ObjectiveState::Completed);
}

bool FPSObjectiveSystem::isFailed(const std::string& objective_id) const {
    return getObjectiveState(objective_id) ==
           static_cast<int>(components::FPSObjective::ObjectiveState::Failed);
}

// ---------------------------------------------------------------------------
// Actions
// ---------------------------------------------------------------------------

bool FPSObjectiveSystem::reportHostileKill(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    using OT = components::FPSObjective::ObjectiveType;
    if (obj->state != static_cast<int>(S::Active)) return false;
    if (obj->objective_type != static_cast<int>(OT::EliminateHostiles)) return false;

    obj->hostiles_killed++;
    if (obj->hostiles_required > 0) {
        obj->progress = std::min(1.0f,
            static_cast<float>(obj->hostiles_killed) /
            static_cast<float>(obj->hostiles_required));
    }
    if (obj->hostiles_killed >= obj->hostiles_required && obj->hostiles_required > 0) {
        obj->state = static_cast<int>(S::Completed);
        obj->progress = 1.0f;
    }
    return true;
}

bool FPSObjectiveSystem::reportItemCollected(
        const std::string& objective_id,
        const std::string& item_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    using OT = components::FPSObjective::ObjectiveType;
    if (obj->state != static_cast<int>(S::Active)) return false;
    if (obj->objective_type != static_cast<int>(OT::RetrieveItem)) return false;
    if (obj->target_item_id != item_id) return false;

    obj->item_collected = true;
    obj->progress = 1.0f;
    obj->state = static_cast<int>(S::Completed);
    return true;
}

bool FPSObjectiveSystem::reportSabotageComplete(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    using OT = components::FPSObjective::ObjectiveType;
    if (obj->state != static_cast<int>(S::Active)) return false;
    if (obj->objective_type != static_cast<int>(OT::Sabotage)) return false;

    obj->progress = 1.0f;
    obj->state = static_cast<int>(S::Completed);
    return true;
}

bool FPSObjectiveSystem::reportExtraction(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    using OT = components::FPSObjective::ObjectiveType;
    if (obj->state != static_cast<int>(S::Active)) return false;
    if (obj->objective_type != static_cast<int>(OT::Escape)) return false;

    obj->progress = 1.0f;
    obj->state = static_cast<int>(S::Completed);
    return true;
}

bool FPSObjectiveSystem::reportVIPRescued(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    using OT = components::FPSObjective::ObjectiveType;
    if (obj->state != static_cast<int>(S::Active)) return false;
    if (obj->objective_type != static_cast<int>(OT::RescueVIP)) return false;

    obj->progress = 1.0f;
    obj->state = static_cast<int>(S::Completed);
    return true;
}

bool FPSObjectiveSystem::reportRepairComplete(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    using OT = components::FPSObjective::ObjectiveType;
    if (obj->state != static_cast<int>(S::Active)) return false;
    if (obj->objective_type != static_cast<int>(OT::RepairSystem)) return false;

    obj->progress = 1.0f;
    obj->state = static_cast<int>(S::Completed);
    return true;
}

bool FPSObjectiveSystem::failObjective(const std::string& objective_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    using S = components::FPSObjective::ObjectiveState;
    if (obj->state != static_cast<int>(S::Active)) return false;
    obj->state = static_cast<int>(S::Failed);
    return true;
}

// ---------------------------------------------------------------------------
// Configuration
// ---------------------------------------------------------------------------

bool FPSObjectiveSystem::setHostileCount(const std::string& objective_id,
                                          int count) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    obj->hostiles_required = count;
    return true;
}

bool FPSObjectiveSystem::setDefendDuration(const std::string& objective_id,
                                            float seconds) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    obj->defend_duration = seconds;
    return true;
}

bool FPSObjectiveSystem::setTargetItem(const std::string& objective_id,
                                        const std::string& item_id) {
    auto* entity = world_->getEntity(objective_id);
    if (!entity) return false;
    auto* obj = entity->getComponent<components::FPSObjective>();
    if (!obj) return false;
    obj->target_item_id = item_id;
    return true;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string FPSObjectiveSystem::objectiveTypeName(int type) {
    return components::FPSObjective::objectiveTypeName(type);
}

std::string FPSObjectiveSystem::stateName(int s) {
    return components::FPSObjective::stateName(s);
}

} // namespace systems
} // namespace atlas
