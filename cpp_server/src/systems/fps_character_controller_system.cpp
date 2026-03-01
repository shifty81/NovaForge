#include "systems/fps_character_controller_system.h"
#include "ecs/world.h"
#include <memory>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FPSCharacterControllerSystem::FPSCharacterControllerSystem(ecs::World* world)
    : System(world) {
}

void FPSCharacterControllerSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FPSCharacterState>();
    for (auto* entity : entities) {
        auto* state = entity->getComponent<components::FPSCharacterState>();
        if (!state) continue;

        // --- Stamina ---
        bool sprinting = state->stance == static_cast<int>(components::FPSCharacterState::Stance::Sprinting);
        if (sprinting && state->stamina > 0.0f) {
            state->stamina = std::max(0.0f, state->stamina - state->stamina_drain * delta_time);
            if (state->stamina <= 0.0f) {
                // Exhausted: fall back to standing
                state->stance = static_cast<int>(components::FPSCharacterState::Stance::Standing);
                sprinting = false;
            }
        }
        if (!sprinting) {
            state->stamina = std::min(state->stamina_max,
                                      state->stamina + state->stamina_regen * delta_time);
        }

        // --- Movement speed ---
        float speed = state->walk_speed;
        if (state->stance == static_cast<int>(components::FPSCharacterState::Stance::Sprinting)) {
            speed = state->sprint_speed;
        } else if (state->stance == static_cast<int>(components::FPSCharacterState::Stance::Crouching)) {
            speed = state->crouch_speed;
        }

        // --- Horizontal movement (rotated by yaw) ---
        float mx = state->move_x;
        float mz = state->move_z;
        float len = std::sqrt(mx * mx + mz * mz);
        if (len > 1.0f) {
            mx /= len;
            mz /= len;
        }
        float yaw_rad = state->yaw * (3.14159265f / 180.0f);
        float cos_yaw = std::cos(yaw_rad);
        float sin_yaw = std::sin(yaw_rad);
        float world_x = mx * cos_yaw - mz * sin_yaw;
        float world_z = mx * sin_yaw + mz * cos_yaw;
        state->pos_x += world_x * speed * delta_time;
        state->pos_z += world_z * speed * delta_time;

        // --- Gravity and vertical movement ---
        if (state->gravity > 0.0f) {
            // Jump
            if (state->jump_requested && state->grounded) {
                state->vel_y = state->jump_impulse;
                state->grounded = false;
            }
            state->jump_requested = false;

            state->vel_y -= state->gravity * delta_time;
            state->pos_y += state->vel_y * delta_time;

            // Floor collision at y = 0
            if (state->pos_y <= 0.0f) {
                state->pos_y = 0.0f;
                state->vel_y = 0.0f;
                state->grounded = true;
            }
        } else {
            // Zero-g: no gravity, vel_y from jump applied directly
            if (state->jump_requested) {
                state->vel_y = state->jump_impulse;
                state->jump_requested = false;
            }
            state->pos_y += state->vel_y * delta_time;
            // In zero-g, never "grounded"
            state->grounded = false;
        }
    }
}

bool FPSCharacterControllerSystem::spawnCharacter(
        const std::string& player_id,
        const std::string& interior_id,
        float x, float y, float z, float yaw) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    if (world_->getEntity(entity_id)) return false;  // already exists

    auto* entity = world_->createEntity(entity_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::FPSCharacterState>();
    comp->player_id = player_id;
    comp->interior_id = interior_id;
    comp->pos_x = x;
    comp->pos_y = y;
    comp->pos_z = z;
    comp->yaw = yaw;
    entity->addComponent(std::move(comp));
    return true;
}

bool FPSCharacterControllerSystem::removeCharacter(const std::string& player_id) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    world_->destroyEntity(entity_id);
    return true;
}

bool FPSCharacterControllerSystem::setMoveInput(const std::string& player_id,
                                                  float move_x, float move_z) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return false;
    state->move_x = std::max(-1.0f, std::min(1.0f, move_x));
    state->move_z = std::max(-1.0f, std::min(1.0f, move_z));
    return true;
}

bool FPSCharacterControllerSystem::setLookDirection(const std::string& player_id,
                                                     float yaw, float pitch) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return false;
    state->yaw = yaw;
    state->pitch = std::max(-89.0f, std::min(89.0f, pitch));
    return true;
}

bool FPSCharacterControllerSystem::requestJump(const std::string& player_id) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return false;
    state->jump_requested = true;
    return true;
}

bool FPSCharacterControllerSystem::setStance(const std::string& player_id, int stance) {
    if (stance < 0 || stance > 2) return false;
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return false;
    state->stance = stance;
    return true;
}

bool FPSCharacterControllerSystem::setGravity(const std::string& player_id, float gravity) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return false;
    state->gravity = std::max(0.0f, gravity);
    return true;
}

std::tuple<float, float, float> FPSCharacterControllerSystem::getPosition(
        const std::string& player_id) const {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return {0, 0, 0};
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return {0, 0, 0};
    return {state->pos_x, state->pos_y, state->pos_z};
}

std::pair<float, float> FPSCharacterControllerSystem::getLookDirection(
        const std::string& player_id) const {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return {0, 0};
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return {0, 0};
    return {state->yaw, state->pitch};
}

bool FPSCharacterControllerSystem::isGrounded(const std::string& player_id) const {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    return state ? state->grounded : false;
}

float FPSCharacterControllerSystem::getStaminaFraction(const std::string& player_id) const {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state || state->stamina_max <= 0.0f) return 0.0f;
    return state->stamina / state->stamina_max;
}

int FPSCharacterControllerSystem::getStance(const std::string& player_id) const {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    return state ? state->stance : 0;
}

std::string FPSCharacterControllerSystem::stanceName(int stance) {
    using Stance = components::FPSCharacterState::Stance;
    switch (static_cast<Stance>(stance)) {
        case Stance::Standing:  return "Standing";
        case Stance::Crouching: return "Crouching";
        case Stance::Sprinting: return "Sprinting";
        default: return "Unknown";
    }
}

bool FPSCharacterControllerSystem::setCurrentRoom(const std::string& player_id,
                                                    const std::string& room_id) {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return false;
    state->current_room_id = room_id;
    return true;
}

std::string FPSCharacterControllerSystem::getCurrentRoom(const std::string& player_id) const {
    std::string entity_id = std::string(FPS_CHAR_PREFIX) + player_id;
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";
    auto* state = entity->getComponent<components::FPSCharacterState>();
    if (!state) return "";
    return state->current_room_id;
}

} // namespace systems
} // namespace atlas
