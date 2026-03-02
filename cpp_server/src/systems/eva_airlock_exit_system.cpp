#include "systems/eva_airlock_exit_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

EVAAirlockExitSystem::EVAAirlockExitSystem(ecs::World* world)
    : System(world) {
}

void EVAAirlockExitSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::EVAAirlockExit>();
    for (auto* entity : entities) {
        auto* exit = entity->getComponent<components::EVAAirlockExit>();
        if (!exit || exit->state == 0) continue;

        using ES = components::EVAAirlockExit::ExitState;
        auto currentState = static_cast<ES>(exit->state);

        // Don't advance InSpace or Complete automatically
        if (currentState == ES::InSpace || currentState == ES::Complete) continue;

        // Check dock-state blocking during CheckingDockState
        if (currentState == ES::CheckingDockState) {
            if (exit->ship_docked) {
                exit->exit_blocked = true;
                exit->state = static_cast<int>(ES::Inactive);
                exit->state_progress = 0.0f;
                exit->player_id.clear();
                continue;
            }
            if (exit->suit_oxygen < exit->min_oxygen) {
                exit->exit_blocked = true;
                exit->state = static_cast<int>(ES::Inactive);
                exit->state_progress = 0.0f;
                exit->player_id.clear();
                continue;
            }
            exit->exit_blocked = false;
        }

        exit->state_progress += delta_time / std::max(exit->state_duration, 0.001f);
        if (exit->state_progress >= 1.0f) {
            exit->state_progress = 1.0f;
            int next = exit->state + 1;
            if (next > static_cast<int>(ES::Complete)) {
                next = static_cast<int>(ES::Complete);
            }
            exit->state = next;
            exit->state_progress = 0.0f;

            auto newState = static_cast<ES>(exit->state);
            if (newState == ES::InSpace) {
                exit->distance_from_ship = 0.0f;
            }
            if (newState == ES::Complete) {
                exit->player_id.clear();
                exit->distance_from_ship = 0.0f;
            }
        }
    }
}

bool EVAAirlockExitSystem::createExitPoint(const std::string& entity_id,
                                            const std::string& ship_id,
                                            float max_tether_range) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    if (entity->getComponent<components::EVAAirlockExit>()) return false;

    auto comp = std::make_unique<components::EVAAirlockExit>();
    comp->airlock_id = entity_id;
    comp->ship_id = ship_id;
    comp->max_tether_range = max_tether_range;
    entity->addComponent(std::move(comp));
    return true;
}

bool EVAAirlockExitSystem::requestExit(const std::string& entity_id,
                                        const std::string& player_id,
                                        float suit_oxygen) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;
    if (exit->state != 0) return false;

    exit->player_id = player_id;
    exit->suit_oxygen = suit_oxygen;
    exit->exit_blocked = false;
    exit->state = static_cast<int>(components::EVAAirlockExit::ExitState::RequestingExit);
    exit->state_progress = 0.0f;
    return true;
}

bool EVAAirlockExitSystem::setDockState(const std::string& entity_id, bool docked) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;

    exit->ship_docked = docked;
    return true;
}

bool EVAAirlockExitSystem::beginReturn(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;

    if (exit->state != static_cast<int>(components::EVAAirlockExit::ExitState::InSpace))
        return false;

    exit->state = static_cast<int>(components::EVAAirlockExit::ExitState::Returning);
    exit->state_progress = 0.0f;
    return true;
}

bool EVAAirlockExitSystem::cancelExit(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;
    if (exit->state == 0) return false;

    exit->state = 0;
    exit->state_progress = 0.0f;
    exit->exit_blocked = false;
    exit->distance_from_ship = 0.0f;
    exit->player_id.clear();
    return true;
}

bool EVAAirlockExitSystem::moveAway(const std::string& entity_id, float distance) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;

    if (exit->state != static_cast<int>(components::EVAAirlockExit::ExitState::InSpace))
        return false;

    exit->distance_from_ship += distance;
    if (exit->tether_active && exit->distance_from_ship > exit->max_tether_range) {
        exit->distance_from_ship = exit->max_tether_range;
    }
    return true;
}

int EVAAirlockExitSystem::getState(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return 0;
    return exit->state;
}

float EVAAirlockExitSystem::getStateProgress(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return 0.0f;
    return exit->state_progress;
}

bool EVAAirlockExitSystem::isExitBlocked(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;
    return exit->exit_blocked;
}

bool EVAAirlockExitSystem::isInSpace(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;
    return exit->state == static_cast<int>(components::EVAAirlockExit::ExitState::InSpace);
}

float EVAAirlockExitSystem::getDistanceFromShip(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return 0.0f;
    return exit->distance_from_ship;
}

bool EVAAirlockExitSystem::isTetherActive(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* exit = entity->getComponent<components::EVAAirlockExit>();
    if (!exit) return false;
    return exit->tether_active;
}

std::string EVAAirlockExitSystem::stateName(int state) {
    using ES = components::EVAAirlockExit::ExitState;
    switch (static_cast<ES>(state)) {
        case ES::Inactive: return "inactive";
        case ES::RequestingExit: return "requesting_exit";
        case ES::CheckingDockState: return "checking_dock_state";
        case ES::PreparingExit: return "preparing_exit";
        case ES::Exiting: return "exiting";
        case ES::InSpace: return "in_space";
        case ES::Returning: return "returning";
        case ES::Complete: return "complete";
    }
    return "unknown";
}

} // namespace systems
} // namespace atlas
