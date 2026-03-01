#include "systems/fps_combat_system.h"
#include "ecs/world.h"
#include <cmath>
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

FPSCombatSystem::FPSCombatSystem(ecs::World* world)
    : System(world) {
}

void FPSCombatSystem::update(float delta_time) {
    // --- Weapon cooldowns and reloading ---
    auto weapons = world_->getEntities<components::FPSWeapon>();
    for (auto* entity : weapons) {
        auto* w = entity->getComponent<components::FPSWeapon>();
        if (!w) continue;

        // Tick cooldown
        if (w->cooldown > 0.0f) {
            w->cooldown = std::max(0.0f, w->cooldown - delta_time);
        }

        // Tick reload
        if (w->is_reloading) {
            float step = delta_time / std::max(0.01f, w->reload_time);
            w->reload_progress += step;
            if (w->reload_progress >= 1.0f) {
                w->reload_progress = 0.0f;
                w->is_reloading = false;
                w->ammo = w->ammo_max;
            }
        }
    }

    // --- Shield recharge for FPSHealth ---
    auto healths = world_->getEntities<components::FPSHealth>();
    for (auto* entity : healths) {
        auto* h = entity->getComponent<components::FPSHealth>();
        if (!h || !h->is_alive) continue;

        h->time_since_last_hit += delta_time;

        if (h->time_since_last_hit >= h->shield_recharge_delay &&
            h->shield < h->shield_max) {
            h->shield = std::min(h->shield_max,
                                 h->shield + h->shield_recharge_rate * delta_time);
        }
    }
}

// ---------------------------------------------------------------------------
// Weapon management
// ---------------------------------------------------------------------------

bool FPSCombatSystem::createWeapon(
        const std::string& weapon_id,
        const std::string& owner_id,
        components::FPSWeapon::WeaponCategory category,
        const std::string& damage_type,
        float damage, float range, float fire_rate,
        int ammo_max, float spread,
        float reload_time) {

    if (world_->getEntity(weapon_id)) return false;

    auto* entity = world_->createEntity(weapon_id);
    if (!entity) return false;

    auto comp = std::make_unique<components::FPSWeapon>();
    comp->weapon_id    = weapon_id;
    comp->owner_id     = owner_id;
    comp->category     = static_cast<int>(category);
    comp->damage_type  = damage_type;
    comp->damage       = damage;
    comp->range        = range;
    comp->fire_rate    = fire_rate;
    comp->ammo         = ammo_max;
    comp->ammo_max     = ammo_max;
    comp->spread       = spread;
    comp->reload_time  = reload_time;
    entity->addComponent(std::move(comp));
    return true;
}

bool FPSCombatSystem::equipWeapon(const std::string& weapon_id) {
    auto* entity = world_->getEntity(weapon_id);
    if (!entity) return false;
    auto* w = entity->getComponent<components::FPSWeapon>();
    if (!w) return false;
    w->is_equipped = true;
    return true;
}

bool FPSCombatSystem::unequipWeapon(const std::string& weapon_id) {
    auto* entity = world_->getEntity(weapon_id);
    if (!entity) return false;
    auto* w = entity->getComponent<components::FPSWeapon>();
    if (!w) return false;
    w->is_equipped = false;
    return true;
}

bool FPSCombatSystem::startReload(const std::string& weapon_id) {
    auto* entity = world_->getEntity(weapon_id);
    if (!entity) return false;
    auto* w = entity->getComponent<components::FPSWeapon>();
    if (!w) return false;
    if (w->is_reloading) return false;          // Already reloading
    if (w->ammo >= w->ammo_max) return false;   // Already full
    w->is_reloading = true;
    w->reload_progress = 0.0f;
    return true;
}

// ---------------------------------------------------------------------------
// Health management
// ---------------------------------------------------------------------------

bool FPSCombatSystem::createHealth(
        const std::string& entity_id,
        float health_max, float shield_max,
        float shield_recharge_rate) {

    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (entity->getComponent<components::FPSHealth>()) return false;

    auto comp = std::make_unique<components::FPSHealth>();
    comp->owner_id             = entity_id;
    comp->health               = health_max;
    comp->health_max           = health_max;
    comp->shield               = shield_max;
    comp->shield_max           = shield_max;
    comp->shield_recharge_rate = shield_recharge_rate;
    entity->addComponent(std::move(comp));
    return true;
}

// ---------------------------------------------------------------------------
// Combat actions
// ---------------------------------------------------------------------------

bool FPSCombatSystem::fireWeapon(
        const std::string& shooter_player_id,
        const std::string& target_entity_id) {

    // Locate shooter's FPS character entity
    std::string char_eid = std::string(FPS_CHAR_PREFIX) + shooter_player_id;
    auto* char_entity = world_->getEntity(char_eid);
    if (!char_entity) return false;
    auto* cs = char_entity->getComponent<components::FPSCharacterState>();
    if (!cs) return false;

    // Find the equipped weapon owned by this player
    components::FPSWeapon* weapon = nullptr;
    ecs::Entity* weapon_entity = nullptr;
    for (auto* ent : world_->getEntities<components::FPSWeapon>()) {
        auto* w = ent->getComponent<components::FPSWeapon>();
        if (w && w->owner_id == shooter_player_id && w->is_equipped) {
            weapon = w;
            weapon_entity = ent;
            break;
        }
    }
    if (!weapon) return false;

    // Weapon checks
    if (weapon->cooldown > 0.0f) return false;
    if (weapon->ammo <= 0) return false;
    if (weapon->is_reloading) return false;

    // Get target position (supports both FPS chars and world entities)
    float tx = 0.0f, ty = 0.0f, tz = 0.0f;
    auto* target_entity = world_->getEntity(target_entity_id);
    if (!target_entity) return false;

    auto* target_cs = target_entity->getComponent<components::FPSCharacterState>();
    if (target_cs) {
        tx = target_cs->pos_x;
        ty = target_cs->pos_y;
        tz = target_cs->pos_z;
    } else {
        auto* target_pos = target_entity->getComponent<components::Position>();
        if (target_pos) {
            tx = target_pos->x;
            ty = target_pos->y;
            tz = target_pos->z;
        } else {
            return false;
        }
    }

    // Range check
    float dx = tx - cs->pos_x;
    float dy = ty - cs->pos_y;
    float dz = tz - cs->pos_z;
    float dist = std::sqrt(dx * dx + dy * dy + dz * dz);

    if (dist > weapon->range) return false;

    // Apply damage
    applyDamage(target_entity_id, weapon->damage, weapon->damage_type);

    // Consume ammo and set cooldown
    weapon->ammo--;
    weapon->cooldown = weapon->fire_rate;

    return true;
}

bool FPSCombatSystem::applyDamage(
        const std::string& target_id, float damage,
        const std::string& /*damage_type*/) {

    auto* entity = world_->getEntity(target_id);
    if (!entity) return false;

    auto* h = entity->getComponent<components::FPSHealth>();
    if (!h || !h->is_alive) return false;

    h->time_since_last_hit = 0.0f;

    // Shield absorbs first
    if (h->shield > 0.0f) {
        h->shield -= damage;
        if (h->shield < 0.0f) {
            damage = -h->shield;
            h->shield = 0.0f;
        } else {
            return true;  // Fully absorbed
        }
    }

    // Remaining goes to health
    h->health -= damage;
    if (h->health <= 0.0f) {
        h->health = 0.0f;
        h->is_alive = false;

        if (death_callback_) {
            // Retrieve position
            float px = 0.0f, py = 0.0f, pz = 0.0f;
            auto* cs = entity->getComponent<components::FPSCharacterState>();
            if (cs) {
                px = cs->pos_x;
                py = cs->pos_y;
                pz = cs->pos_z;
            }
            death_callback_(target_id, px, py, pz);
        }
    }

    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

float FPSCombatSystem::getHealth(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* h = entity->getComponent<components::FPSHealth>();
    return h ? h->health : 0.0f;
}

float FPSCombatSystem::getShield(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* h = entity->getComponent<components::FPSHealth>();
    return h ? h->shield : 0.0f;
}

bool FPSCombatSystem::isAlive(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* h = entity->getComponent<components::FPSHealth>();
    return h ? h->is_alive : false;
}

int FPSCombatSystem::getAmmo(const std::string& weapon_id) const {
    auto* entity = world_->getEntity(weapon_id);
    if (!entity) return 0;
    auto* w = entity->getComponent<components::FPSWeapon>();
    return w ? w->ammo : 0;
}

bool FPSCombatSystem::isReloading(const std::string& weapon_id) const {
    auto* entity = world_->getEntity(weapon_id);
    if (!entity) return false;
    auto* w = entity->getComponent<components::FPSWeapon>();
    return w ? w->is_reloading : false;
}

std::string FPSCombatSystem::categoryName(int category) {
    using C = components::FPSWeapon::WeaponCategory;
    switch (static_cast<C>(category)) {
        case C::Sidearm:  return "Sidearm";
        case C::Rifle:    return "Rifle";
        case C::Shotgun:  return "Shotgun";
        case C::Tool:     return "Tool";
        default: return "Unknown";
    }
}

} // namespace systems
} // namespace atlas
