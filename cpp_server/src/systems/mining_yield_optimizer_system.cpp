#include "systems/mining_yield_optimizer_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/economy_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

MiningYieldOptimizerSystem::MiningYieldOptimizerSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void MiningYieldOptimizerSystem::updateComponent(ecs::Entity& entity,
    components::MiningYieldState& mys, float delta_time) {
    if (!mys.active) return;

    // Recalculate final multiplier each tick
    mys.final_multiplier = (1.0f + mys.skill_bonus)
                         * (1.0f + mys.module_bonus)
                         * (1.0f + mys.environment_bonus)
                         * mys.security_modifier;
    mys.final_multiplier = std::max(0.0f, mys.final_multiplier);
    mys.elapsed += delta_time;
}

bool MiningYieldOptimizerSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::MiningYieldState>();
    entity->addComponent(std::move(comp));
    return true;
}

bool MiningYieldOptimizerSystem::setSkillBonus(const std::string& entity_id, float bonus) {
    auto* mys = getComponentFor(entity_id);
    if (!mys) return false;
    mys->skill_bonus = std::max(0.0f, bonus);
    return true;
}

bool MiningYieldOptimizerSystem::setModuleBonus(const std::string& entity_id, float bonus) {
    auto* mys = getComponentFor(entity_id);
    if (!mys) return false;
    mys->module_bonus = std::max(0.0f, bonus);
    return true;
}

bool MiningYieldOptimizerSystem::setEnvironmentBonus(const std::string& entity_id, float bonus) {
    auto* mys = getComponentFor(entity_id);
    if (!mys) return false;
    mys->environment_bonus = std::max(0.0f, bonus);
    return true;
}

bool MiningYieldOptimizerSystem::setSecurityModifier(const std::string& entity_id, float security) {
    auto* mys = getComponentFor(entity_id);
    if (!mys) return false;
    mys->security_modifier = std::max(0.1f, std::min(2.0f, security));
    return true;
}

bool MiningYieldOptimizerSystem::recordCycle(const std::string& entity_id, float base_yield) {
    auto* mys = getComponentFor(entity_id);
    if (!mys || base_yield <= 0.0f) return false;

    float actual_yield = base_yield * mys->final_multiplier;
    mys->total_yield += actual_yield;
    mys->cycle_count++;
    return true;
}

float MiningYieldOptimizerSystem::getSkillBonus(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    return mys ? mys->skill_bonus : 0.0f;
}

float MiningYieldOptimizerSystem::getModuleBonus(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    return mys ? mys->module_bonus : 0.0f;
}

float MiningYieldOptimizerSystem::getEnvironmentBonus(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    return mys ? mys->environment_bonus : 0.0f;
}

float MiningYieldOptimizerSystem::getFinalMultiplier(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    return mys ? mys->final_multiplier : 0.0f;
}

float MiningYieldOptimizerSystem::getTotalYield(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    return mys ? mys->total_yield : 0.0f;
}

int MiningYieldOptimizerSystem::getCycleCount(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    return mys ? mys->cycle_count : 0;
}

float MiningYieldOptimizerSystem::getAverageYieldPerCycle(const std::string& entity_id) const {
    auto* mys = getComponentFor(entity_id);
    if (!mys || mys->cycle_count == 0) return 0.0f;
    return mys->total_yield / static_cast<float>(mys->cycle_count);
}

} // namespace systems
} // namespace atlas
