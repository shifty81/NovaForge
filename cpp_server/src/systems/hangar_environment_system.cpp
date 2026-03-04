#include "systems/hangar_environment_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

HangarEnvironmentSystem::HangarEnvironmentSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void HangarEnvironmentSystem::updateComponent(ecs::Entity& entity, components::HangarEnvironment& comp, float delta_time) {
    using AT = components::HangarEnvironment::AtmosphereType;

    if (comp.is_hangar_open) {
        comp.total_exposure_time += delta_time;

        // Mix external atmosphere based on type
        switch (comp.atmosphere_type) {
            case AT::Toxic:
                comp.current_toxicity += comp.atmosphere_mix_rate * delta_time;
                comp.current_toxicity = std::min(1.0f, comp.current_toxicity);
                break;
            case AT::Corrosive:
                comp.current_corrosion += comp.atmosphere_mix_rate * delta_time;
                comp.current_corrosion = std::min(1.0f, comp.current_corrosion);
                break;
            case AT::Extreme:
                comp.current_toxicity += comp.atmosphere_mix_rate * delta_time;
                comp.current_toxicity = std::min(1.0f, comp.current_toxicity);
                comp.current_corrosion += comp.atmosphere_mix_rate * delta_time;
                comp.current_corrosion = std::min(1.0f, comp.current_corrosion);
                break;
            case AT::None:
                // Rapid depressurization
                comp.internal_pressure -= comp.atmosphere_mix_rate * delta_time * 2.0f;
                comp.internal_pressure = std::max(0.0f, comp.internal_pressure);
                break;
            case AT::Breathable:
            default:
                break;
        }

        // Temperature shifts toward external
        float temp_diff = comp.external_temperature - comp.internal_temperature;
        comp.internal_temperature += temp_diff * comp.atmosphere_mix_rate * delta_time * 0.5f;

        // Alarm activates when toxicity > 0.3 or corrosion > 0.3
        comp.is_alarm_active = (comp.current_toxicity > 0.3f || comp.current_corrosion > 0.3f);

    } else {
        // Recovery when closed
        float recovery_rate = 0.05f;

        comp.current_toxicity = std::max(0.0f, comp.current_toxicity - recovery_rate * delta_time);
        comp.current_corrosion = std::max(0.0f, comp.current_corrosion - recovery_rate * delta_time);

        // Temperature returns toward 22.0
        float temp_diff = 22.0f - comp.internal_temperature;
        comp.internal_temperature += temp_diff * recovery_rate * delta_time;

        // Pressure recovery
        if (comp.internal_pressure < 1.0f) {
            comp.internal_pressure += recovery_rate * delta_time;
            comp.internal_pressure = std::min(1.0f, comp.internal_pressure);
        }

        // Alarm deactivates when toxicity < 0.1 and corrosion < 0.1
        if (comp.current_toxicity < 0.1f && comp.current_corrosion < 0.1f) {
            comp.is_alarm_active = false;
        }
    }
}

bool HangarEnvironmentSystem::initializeEnvironment(
        const std::string& entity_id,
        components::HangarEnvironment::AtmosphereType atmosphere_type,
        float external_temp, float external_pressure) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::HangarEnvironment>();
    if (existing) return false;

    auto comp = std::make_unique<components::HangarEnvironment>();
    comp->hangar_entity_id = entity_id;
    comp->atmosphere_type = atmosphere_type;
    comp->external_temperature = external_temp;
    comp->external_pressure = external_pressure;
    entity->addComponent(std::move(comp));
    return true;
}

bool HangarEnvironmentSystem::openHangar(const std::string& entity_id) {
    auto* env = getComponentFor(entity_id);
    if (!env) return false;

    env->is_hangar_open = true;
    return true;
}

bool HangarEnvironmentSystem::closeHangar(const std::string& entity_id) {
    auto* env = getComponentFor(entity_id);
    if (!env) return false;

    env->is_hangar_open = false;
    return true;
}

bool HangarEnvironmentSystem::addOccupant(const std::string& entity_id,
                                           const std::string& occupant_id,
                                           bool has_suit, float suit_rating) {
    auto* env = getComponentFor(entity_id);
    if (!env) return false;

    // Check not already present
    for (const auto& o : env->occupants) {
        if (o.entity_id == occupant_id) return false;
    }

    components::HangarEnvironment::OccupantInfo occupant;
    occupant.entity_id = occupant_id;
    occupant.has_suit = has_suit;
    occupant.suit_rating = std::max(0.0f, std::min(1.0f, suit_rating));
    env->occupants.push_back(occupant);
    return true;
}

bool HangarEnvironmentSystem::removeOccupant(const std::string& entity_id,
                                              const std::string& occupant_id) {
    auto* env = getComponentFor(entity_id);
    if (!env) return false;

    for (auto it = env->occupants.begin(); it != env->occupants.end(); ++it) {
        if (it->entity_id == occupant_id) {
            env->occupants.erase(it);
            return true;
        }
    }
    return false;
}

bool HangarEnvironmentSystem::setAtmosphere(const std::string& entity_id,
                                             components::HangarEnvironment::AtmosphereType atmosphere_type) {
    auto* env = getComponentFor(entity_id);
    if (!env) return false;

    env->atmosphere_type = atmosphere_type;
    return true;
}

float HangarEnvironmentSystem::getOccupantDamage(const std::string& entity_id,
                                                   const std::string& occupant_id) const {
    const auto* env = getComponentFor(entity_id);
    if (!env) return 0.0f;

    for (const auto& o : env->occupants) {
        if (o.entity_id == occupant_id) {
            float hazard = std::max(env->current_toxicity, env->current_corrosion);
            float damage = env->damage_per_tick * hazard;
            if (o.has_suit) {
                damage *= (1.0f - o.suit_rating);
            }
            return damage;
        }
    }
    return 0.0f;
}

float HangarEnvironmentSystem::getToxicity(const std::string& entity_id) const {
    const auto* env = getComponentFor(entity_id);
    if (!env) return 0.0f;

    return env->current_toxicity;
}

bool HangarEnvironmentSystem::isAlarmActive(const std::string& entity_id) const {
    const auto* env = getComponentFor(entity_id);
    if (!env) return false;

    return env->is_alarm_active;
}

int HangarEnvironmentSystem::getOccupantCount(const std::string& entity_id) const {
    const auto* env = getComponentFor(entity_id);
    if (!env) return 0;

    return static_cast<int>(env->occupants.size());
}

} // namespace systems
} // namespace atlas
