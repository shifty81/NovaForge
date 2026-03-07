#include "systems/damage_resistance_profile_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

DamageResistanceProfileSystem::DamageResistanceProfileSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void DamageResistanceProfileSystem::updateComponent(ecs::Entity& entity,
    components::DamageResistanceProfile& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Active hardeners cycle each tick — drain charge proportional to bonus
    for (auto& h : comp.hardeners) {
        if (h.is_active) {
            comp.charge_consumed += std::abs(h.bonus) * delta_time * 10.0f;
        }
    }
}

bool DamageResistanceProfileSystem::initialize(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::DamageResistanceProfile>();
    entity->addComponent(std::move(comp));
    return true;
}

bool DamageResistanceProfileSystem::setBaseResistance(const std::string& entity_id,
    const std::string& damage_type, float value) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    // Validate damage type
    if (damage_type != "em" && damage_type != "thermal" &&
        damage_type != "kinetic" && damage_type != "explosive") return false;

    // Clamp 0.0–0.85
    value = std::max(0.0f, std::min(0.85f, value));

    if (damage_type == "em") comp->base_em = value;
    else if (damage_type == "thermal") comp->base_thermal = value;
    else if (damage_type == "kinetic") comp->base_kinetic = value;
    else comp->base_explosive = value;

    return true;
}

bool DamageResistanceProfileSystem::addHardener(const std::string& entity_id,
    const std::string& hardener_id, const std::string& damage_type, float bonus) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    if (damage_type != "em" && damage_type != "thermal" &&
        damage_type != "kinetic" && damage_type != "explosive") return false;

    // No duplicate IDs
    for (const auto& h : comp->hardeners) {
        if (h.hardener_id == hardener_id) return false;
    }
    if (static_cast<int>(comp->hardeners.size()) >= comp->max_hardeners) return false;

    components::DamageResistanceProfile::Hardener h;
    h.hardener_id = hardener_id;
    h.damage_type = damage_type;
    h.bonus = std::max(0.0f, std::min(0.5f, bonus));  // cap individual bonus at 50%
    comp->hardeners.push_back(h);
    return true;
}

bool DamageResistanceProfileSystem::removeHardener(const std::string& entity_id,
    const std::string& hardener_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->hardeners.begin(), comp->hardeners.end(),
        [&hardener_id](const components::DamageResistanceProfile::Hardener& h) {
            return h.hardener_id == hardener_id;
        });
    if (it == comp->hardeners.end()) return false;
    comp->hardeners.erase(it);
    return true;
}

bool DamageResistanceProfileSystem::activateHardener(const std::string& entity_id,
    const std::string& hardener_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& h : comp->hardeners) {
        if (h.hardener_id == hardener_id) {
            if (h.is_active) return false;  // already active
            h.is_active = true;
            return true;
        }
    }
    return false;
}

bool DamageResistanceProfileSystem::deactivateHardener(const std::string& entity_id,
    const std::string& hardener_id) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    for (auto& h : comp->hardeners) {
        if (h.hardener_id == hardener_id) {
            if (!h.is_active) return false;  // already inactive
            h.is_active = false;
            return true;
        }
    }
    return false;
}

float DamageResistanceProfileSystem::applyResistance(const std::string& entity_id,
    const std::string& damage_type, float raw_damage) {
    auto* comp = getComponentFor(entity_id);
    if (!comp || raw_damage <= 0.0f) return raw_damage;

    float effective = getEffectiveResistance(entity_id, damage_type);
    float mitigated = raw_damage * effective;
    comp->total_damage_mitigated += mitigated;
    return raw_damage - mitigated;
}

float DamageResistanceProfileSystem::getEffectiveResistance(const std::string& entity_id,
    const std::string& damage_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;

    // Get base resistance for this type
    float base = 0.0f;
    if (damage_type == "em") base = comp->base_em;
    else if (damage_type == "thermal") base = comp->base_thermal;
    else if (damage_type == "kinetic") base = comp->base_kinetic;
    else if (damage_type == "explosive") base = comp->base_explosive;

    // Apply active hardener bonuses with stacking penalty
    // Stacking penalty: nth bonus is multiplied by e^(-(n-1)^2 / 7.1289)
    std::vector<float> bonuses;
    for (const auto& h : comp->hardeners) {
        if (h.is_active && h.damage_type == damage_type) {
            bonuses.push_back(h.bonus);
        }
    }
    // Sort descending so largest bonus gets least penalty
    std::sort(bonuses.begin(), bonuses.end(), std::greater<float>());

    float total_bonus = 0.0f;
    for (size_t i = 0; i < bonuses.size(); i++) {
        float penalty = std::exp(-static_cast<float>(i * i) / 7.1289f);
        total_bonus += bonuses[i] * penalty;
    }

    float effective = base + total_bonus * (1.0f - base);
    return std::min(0.85f, effective);  // Hard cap at 85%
}

int DamageResistanceProfileSystem::getHardenerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->hardeners.size()) : 0;
}

int DamageResistanceProfileSystem::getActiveHardenerCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0;
    int count = 0;
    for (const auto& h : comp->hardeners) {
        if (h.is_active) count++;
    }
    return count;
}

float DamageResistanceProfileSystem::getTotalDamageMitigated(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_damage_mitigated : 0.0f;
}

} // namespace systems
} // namespace atlas
