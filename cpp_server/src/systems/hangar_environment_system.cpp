#include "systems/hangar_environment_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

HangarEnvironmentSystem::HangarEnvironmentSystem(ecs::World* world)
    : System(world) {
}

void HangarEnvironmentSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::HangarEnvironment>();
    for (auto* entity : entities) {
        auto* env = entity->getComponent<components::HangarEnvironment>();
        if (!env) continue;

        using AT = components::HangarEnvironment::AtmosphereType;

        if (env->is_hangar_open) {
            env->total_exposure_time += delta_time;

            // Mix external atmosphere based on type
            switch (env->atmosphere_type) {
                case AT::Toxic:
                    env->current_toxicity += env->atmosphere_mix_rate * delta_time;
                    env->current_toxicity = std::min(1.0f, env->current_toxicity);
                    break;
                case AT::Corrosive:
                    env->current_corrosion += env->atmosphere_mix_rate * delta_time;
                    env->current_corrosion = std::min(1.0f, env->current_corrosion);
                    break;
                case AT::Extreme:
                    env->current_toxicity += env->atmosphere_mix_rate * delta_time;
                    env->current_toxicity = std::min(1.0f, env->current_toxicity);
                    env->current_corrosion += env->atmosphere_mix_rate * delta_time;
                    env->current_corrosion = std::min(1.0f, env->current_corrosion);
                    break;
                case AT::None:
                    // Rapid depressurization
                    env->internal_pressure -= env->atmosphere_mix_rate * delta_time * 2.0f;
                    env->internal_pressure = std::max(0.0f, env->internal_pressure);
                    break;
                case AT::Breathable:
                default:
                    break;
            }

            // Temperature shifts toward external
            float temp_diff = env->external_temperature - env->internal_temperature;
            env->internal_temperature += temp_diff * env->atmosphere_mix_rate * delta_time * 0.5f;

            // Alarm activates when toxicity > 0.3 or corrosion > 0.3
            env->is_alarm_active = (env->current_toxicity > 0.3f || env->current_corrosion > 0.3f);

        } else {
            // Recovery when closed
            float recovery_rate = 0.05f;

            env->current_toxicity = std::max(0.0f, env->current_toxicity - recovery_rate * delta_time);
            env->current_corrosion = std::max(0.0f, env->current_corrosion - recovery_rate * delta_time);

            // Temperature returns toward 22.0
            float temp_diff = 22.0f - env->internal_temperature;
            env->internal_temperature += temp_diff * recovery_rate * delta_time;

            // Pressure recovery
            if (env->internal_pressure < 1.0f) {
                env->internal_pressure += recovery_rate * delta_time;
                env->internal_pressure = std::min(1.0f, env->internal_pressure);
            }

            // Alarm deactivates when toxicity < 0.1 and corrosion < 0.1
            if (env->current_toxicity < 0.1f && env->current_corrosion < 0.1f) {
                env->is_alarm_active = false;
            }
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
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* env = entity->getComponent<components::HangarEnvironment>();
    if (!env) return false;

    env->is_hangar_open = true;
    return true;
}

bool HangarEnvironmentSystem::closeHangar(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* env = entity->getComponent<components::HangarEnvironment>();
    if (!env) return false;

    env->is_hangar_open = false;
    return true;
}

bool HangarEnvironmentSystem::addOccupant(const std::string& entity_id,
                                           const std::string& occupant_id,
                                           bool has_suit, float suit_rating) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* env = entity->getComponent<components::HangarEnvironment>();
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
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* env = entity->getComponent<components::HangarEnvironment>();
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
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* env = entity->getComponent<components::HangarEnvironment>();
    if (!env) return false;

    env->atmosphere_type = atmosphere_type;
    return true;
}

float HangarEnvironmentSystem::getOccupantDamage(const std::string& entity_id,
                                                   const std::string& occupant_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* env = entity->getComponent<components::HangarEnvironment>();
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
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* env = entity->getComponent<components::HangarEnvironment>();
    if (!env) return 0.0f;

    return env->current_toxicity;
}

bool HangarEnvironmentSystem::isAlarmActive(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* env = entity->getComponent<components::HangarEnvironment>();
    if (!env) return false;

    return env->is_alarm_active;
}

int HangarEnvironmentSystem::getOccupantCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* env = entity->getComponent<components::HangarEnvironment>();
    if (!env) return 0;

    return static_cast<int>(env->occupants.size());
}

} // namespace systems
} // namespace atlas
