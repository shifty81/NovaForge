#include "systems/abyssal_weather_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

AbyssalWeatherSystem::AbyssalWeatherSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void AbyssalWeatherSystem::updateComponent(ecs::Entity& /*entity*/,
    components::AbyssalWeatherState& comp, float delta_time) {
    if (!comp.active) return;
    comp.elapsed += delta_time;
    // Effect is static for the pocket lifetime – nothing to tick.
}

// ---------------------------------------------------------------------------
// Static helper: build weather modifiers from type + intensity
// ---------------------------------------------------------------------------
components::AbyssalWeatherState::WeatherEffect
AbyssalWeatherSystem::buildEffect(
    components::AbyssalWeatherState::WeatherType type, float intensity) {
    using WT = components::AbyssalWeatherState::WeatherType;
    components::AbyssalWeatherState::WeatherEffect e;
    // scale: 1 + (intensity-1)*0.2  → 1.0 at T1, 1.8 at T5
    float boost  = 1.0f + (intensity - 1.0f) * 0.2f;
    float debuff = 1.0f - (intensity - 1.0f) * 0.1f;

    switch (type) {
    case WT::Electrical:
        e.turret_optimal_modifier      = boost;
        e.capacitor_recharge_modifier  = debuff;
        break;
    case WT::DarkMatter:
        e.drone_speed_modifier         = debuff;
        e.visibility_modifier          = debuff;
        break;
    case WT::ExoticPlasma:
        e.missile_velocity_modifier    = boost;
        e.armor_hp_modifier            = debuff;
        break;
    case WT::Gamma:
        e.shield_hp_modifier           = boost;
        e.hull_hp_modifier             = debuff;
        break;
    case WT::Firestorm:
        e.ew_strength_modifier         = boost;
        e.propulsion_modifier          = debuff;
        break;
    default:
        break;
    }
    return e;
}

// ---------------------------------------------------------------------------
// Public API
// ---------------------------------------------------------------------------

bool AbyssalWeatherSystem::initialize(const std::string& entity_id,
    const std::string& pocket_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::AbyssalWeatherState>();
    comp->pocket_id = pocket_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool AbyssalWeatherSystem::setWeather(const std::string& entity_id,
    components::AbyssalWeatherState::WeatherType type, int tier) {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return false;
    comp->current_weather = type;
    comp->tier = (std::max)(1, (std::min)(tier, 5));
    comp->intensity = static_cast<float>(comp->tier);
    comp->effect = buildEffect(type, comp->intensity);
    return true;
}

components::AbyssalWeatherState::WeatherType
AbyssalWeatherSystem::getWeather(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->current_weather
                : components::AbyssalWeatherState::WeatherType::None;
}

float AbyssalWeatherSystem::getIntensity(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->intensity : 0.0f;
}

components::AbyssalWeatherState::WeatherEffect
AbyssalWeatherSystem::getEffect(const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    if (!comp) return {};
    return comp->effect;
}

float AbyssalWeatherSystem::getTurretOptimalModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.turret_optimal_modifier : 1.0f;
}

float AbyssalWeatherSystem::getMissileVelocityModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.missile_velocity_modifier : 1.0f;
}

float AbyssalWeatherSystem::getDroneSpeedModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.drone_speed_modifier : 1.0f;
}

float AbyssalWeatherSystem::getShieldHpModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.shield_hp_modifier : 1.0f;
}

float AbyssalWeatherSystem::getArmorHpModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.armor_hp_modifier : 1.0f;
}

float AbyssalWeatherSystem::getCapacitorRechargeModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.capacitor_recharge_modifier : 1.0f;
}

float AbyssalWeatherSystem::getPropulsionModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.propulsion_modifier : 1.0f;
}

float AbyssalWeatherSystem::getEwStrengthModifier(
    const std::string& entity_id) const {
    auto* comp = getComponentFor(entity_id);
    return comp ? comp->effect.ew_strength_modifier : 1.0f;
}

} // namespace systems
} // namespace atlas
