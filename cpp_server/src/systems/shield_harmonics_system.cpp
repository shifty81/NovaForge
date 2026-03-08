#include "systems/shield_harmonics_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

ShieldHarmonicsSystem::ShieldHarmonicsSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void ShieldHarmonicsSystem::updateComponent(ecs::Entity& /*entity*/,
    components::ShieldHarmonicsState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;

    // Gradually move frequency toward optimal
    if (std::fabs(comp.frequency - comp.optimal_frequency) > 0.01f) {
        float dir = (comp.optimal_frequency > comp.frequency) ? 1.0f : -1.0f;
        float step = comp.tuning_speed * delta_time;
        float remaining = std::fabs(comp.optimal_frequency - comp.frequency);
        comp.frequency += dir * std::min(step, remaining);
        comp.frequency = std::max(0.0f, std::min(100.0f, comp.frequency));
    }

    // Recalculate resonance: how close frequency is to optimal (0-1)
    float dist = std::fabs(comp.frequency - comp.optimal_frequency);
    comp.resonance_strength = std::max(0.0f, 1.0f - dist / 50.0f);

    // Update effective resistances
    for (auto& profile : comp.profiles) {
        profile.tuned_bonus = comp.resonance_strength * comp.max_bonus;
        profile.effective_resistance = std::min(1.0f,
            profile.base_resistance + profile.tuned_bonus);
    }
}

bool ShieldHarmonicsSystem::initialize(const std::string& entity_id,
    float initial_frequency) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    if (initial_frequency < 0.0f || initial_frequency > 100.0f) return false;

    auto comp = std::make_unique<components::ShieldHarmonicsState>();
    comp->frequency = initial_frequency;
    comp->optimal_frequency = initial_frequency;
    comp->resonance_strength = 1.0f; // Fully aligned at init
    entity->addComponent(std::move(comp));
    return true;
}

bool ShieldHarmonicsSystem::addProfile(const std::string& entity_id,
    const std::string& damage_type, float base_resistance) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (damage_type.empty()) return false;
    if (base_resistance < 0.0f || base_resistance > 1.0f) return false;

    // Check for duplicate
    for (const auto& p : comp->profiles) {
        if (p.damage_type == damage_type) return false;
    }

    if (static_cast<int>(comp->profiles.size()) >= comp->max_profiles) return false;

    components::ShieldHarmonicsState::HarmonicProfile profile;
    profile.damage_type = damage_type;
    profile.base_resistance = base_resistance;
    profile.effective_resistance = base_resistance;
    comp->profiles.push_back(profile);
    return true;
}

bool ShieldHarmonicsSystem::removeProfile(const std::string& entity_id,
    const std::string& damage_type) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;

    auto it = std::find_if(comp->profiles.begin(), comp->profiles.end(),
        [&damage_type](const components::ShieldHarmonicsState::HarmonicProfile& p) {
            return p.damage_type == damage_type;
        });
    if (it == comp->profiles.end()) return false;

    comp->profiles.erase(it);
    return true;
}

bool ShieldHarmonicsSystem::tuneFrequency(const std::string& entity_id,
    float target_frequency) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (target_frequency < 0.0f || target_frequency > 100.0f) return false;

    comp->optimal_frequency = target_frequency;
    comp->total_retunings++;
    return true;
}

bool ShieldHarmonicsSystem::setTuningSpeed(const std::string& entity_id, float speed) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (speed <= 0.0f) return false;

    comp->tuning_speed = speed;
    return true;
}

bool ShieldHarmonicsSystem::setMaxBonus(const std::string& entity_id, float bonus) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    if (bonus < 0.0f || bonus > 1.0f) return false;

    comp->max_bonus = bonus;
    return true;
}

float ShieldHarmonicsSystem::getFrequency(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->frequency : 0.0f;
}

float ShieldHarmonicsSystem::getResonanceStrength(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->resonance_strength : 0.0f;
}

float ShieldHarmonicsSystem::getEffectiveResistance(const std::string& entity_id,
    const std::string& damage_type) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return 0.0f;
    for (const auto& p : comp->profiles) {
        if (p.damage_type == damage_type) return p.effective_resistance;
    }
    return 0.0f;
}

int ShieldHarmonicsSystem::getProfileCount(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? static_cast<int>(comp->profiles.size()) : 0;
}

float ShieldHarmonicsSystem::getTuningSpeed(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->tuning_speed : 0.0f;
}

float ShieldHarmonicsSystem::getMaxBonus(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->max_bonus : 0.0f;
}

int ShieldHarmonicsSystem::getTotalRetunings(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->total_retunings : 0;
}

} // namespace systems
} // namespace atlas
