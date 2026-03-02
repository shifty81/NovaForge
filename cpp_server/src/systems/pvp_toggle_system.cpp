#include "systems/pvp_toggle_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

PvPToggleSystem::PvPToggleSystem(ecs::World* world)
    : System(world) {
}

void PvPToggleSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::PvPState>();
    for (auto* entity : entities) {
        auto* ps = entity->getComponent<components::PvPState>();
        if (!ps || !ps->active) continue;

        if (ps->aggression_timer > 0.0f) {
            ps->aggression_timer = std::max(0.0f, ps->aggression_timer - delta_time);
        }
    }
}

bool PvPToggleSystem::createPvPState(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PvPState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool PvPToggleSystem::enablePvP(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return false;
    if (ps->safety_level == "HighSec") return false;
    ps->pvp_enabled = true;
    return true;
}

bool PvPToggleSystem::disablePvP(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return false;
    if (ps->aggression_timer > 0.0f) return false;
    if (ps->safety_level == "NullSec" || ps->safety_level == "Wormhole") return false;
    ps->pvp_enabled = false;
    return true;
}

bool PvPToggleSystem::setSafetyLevel(const std::string& entity_id, const std::string& level) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return false;
    if (level != "HighSec" && level != "LowSec" && level != "NullSec" && level != "Wormhole")
        return false;
    ps->safety_level = level;
    if (level == "HighSec") {
        ps->pvp_enabled = false;
    } else if (level == "NullSec" || level == "Wormhole") {
        ps->pvp_enabled = true;
    }
    return true;
}

bool PvPToggleSystem::canEngage(const std::string& attacker_id,
                                const std::string& defender_id) const {
    auto* a_entity = world_->getEntity(attacker_id);
    auto* d_entity = world_->getEntity(defender_id);
    if (!a_entity || !d_entity) return false;
    auto* a_ps = a_entity->getComponent<components::PvPState>();
    auto* d_ps = d_entity->getComponent<components::PvPState>();
    if (!a_ps || !d_ps) return false;
    // HighSec: no PvP ever
    if (a_ps->safety_level == "HighSec" || d_ps->safety_level == "HighSec") return false;
    // NullSec / Wormhole: always engageable
    if (a_ps->safety_level == "NullSec" || a_ps->safety_level == "Wormhole") return true;
    // LowSec: both must be flagged
    return a_ps->pvp_enabled && d_ps->pvp_enabled;
}

bool PvPToggleSystem::recordEngagement(const std::string& attacker_id,
                                       const std::string& defender_id) {
    auto* a_entity = world_->getEntity(attacker_id);
    auto* d_entity = world_->getEntity(defender_id);
    if (!a_entity || !d_entity) return false;
    auto* a_ps = a_entity->getComponent<components::PvPState>();
    auto* d_ps = d_entity->getComponent<components::PvPState>();
    if (!a_ps || !d_ps) return false;
    a_ps->aggression_timer = a_ps->engagement_timer;
    d_ps->aggression_timer = d_ps->engagement_timer;
    a_ps->last_target = defender_id;
    d_ps->last_target = attacker_id;
    return true;
}

bool PvPToggleSystem::recordKill(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return false;
    ps->kill_count++;
    ps->bounty += 1000.0f;
    ps->security_status = std::max(-10.0f, ps->security_status - 0.5f);
    return true;
}

int PvPToggleSystem::getKillCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return 0;
    return ps->kill_count;
}

float PvPToggleSystem::getSecurityStatus(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return 0.0f;
    return ps->security_status;
}

float PvPToggleSystem::getAggressionTimer(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return 0.0f;
    return ps->aggression_timer;
}

bool PvPToggleSystem::isPvPEnabled(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return false;
    return ps->pvp_enabled;
}

std::string PvPToggleSystem::getSafetyLevel(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return "";
    return ps->safety_level;
}

float PvPToggleSystem::getBounty(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ps = entity->getComponent<components::PvPState>();
    if (!ps) return 0.0f;
    return ps->bounty;
}

} // namespace systems
} // namespace atlas
