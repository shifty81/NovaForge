#include "systems/terraforming_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

TerraformingSystem::TerraformingSystem(ecs::World* world)
    : StateMachineSystem(world) {
}

static int stageIndex(components::Terraforming::TerraformStage stage) {
    switch (stage) {
        case components::Terraforming::TerraformStage::Planning: return 0;
        case components::Terraforming::TerraformStage::Infrastructure: return 1;
        case components::Terraforming::TerraformStage::AtmosphereProcessing: return 2;
        case components::Terraforming::TerraformStage::TemperatureRegulation: return 3;
        case components::Terraforming::TerraformStage::BiomeSeeding: return 4;
        case components::Terraforming::TerraformStage::Complete: return 5;
        default: return 0;
    }
}

static components::Terraforming::TerraformStage nextStage(components::Terraforming::TerraformStage stage) {
    switch (stage) {
        case components::Terraforming::TerraformStage::Planning: return components::Terraforming::TerraformStage::Infrastructure;
        case components::Terraforming::TerraformStage::Infrastructure: return components::Terraforming::TerraformStage::AtmosphereProcessing;
        case components::Terraforming::TerraformStage::AtmosphereProcessing: return components::Terraforming::TerraformStage::TemperatureRegulation;
        case components::Terraforming::TerraformStage::TemperatureRegulation: return components::Terraforming::TerraformStage::BiomeSeeding;
        case components::Terraforming::TerraformStage::BiomeSeeding: return components::Terraforming::TerraformStage::Complete;
        default: return components::Terraforming::TerraformStage::Complete;
    }
}

void TerraformingSystem::updateComponent(ecs::Entity& /*entity*/, components::Terraforming& tf, float delta_time) {
    if (!tf.is_active) return;
    if (tf.stage == components::Terraforming::TerraformStage::Complete) return;

    tf.elapsed_in_stage += delta_time;
    tf.total_credits_spent += static_cast<double>(tf.resource_cost_per_tick);

    // Auto-advance when stage time exceeded
    if (tf.elapsed_in_stage >= tf.time_per_stage) {
        tf.stage = nextStage(tf.stage);
        tf.elapsed_in_stage = 0.0f;
    }

    // Progress within current stage
    if (tf.stage != components::Terraforming::TerraformStage::Complete) {
        tf.progress = std::clamp(tf.elapsed_in_stage / tf.time_per_stage, 0.0f, 1.0f);
    } else {
        tf.progress = 1.0f;
    }

    // Total progress across all stages
    int si = stageIndex(tf.stage);
    tf.total_progress = std::clamp((static_cast<float>(si) + tf.progress) / 5.0f, 0.0f, 1.0f);

    // Move environment params toward targets proportional to total_progress
    tf.current_atmosphere = tf.current_atmosphere + (tf.atmosphere_target - tf.current_atmosphere) * delta_time / tf.time_per_stage;
    tf.current_temperature = tf.current_temperature + (tf.temperature_target - tf.current_temperature) * delta_time / tf.time_per_stage;
    tf.current_water_coverage = tf.current_water_coverage + (tf.water_coverage_target - tf.current_water_coverage) * delta_time / tf.time_per_stage;
}

bool TerraformingSystem::startTerraforming(const std::string& entity_id, const std::string& planet_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::Terraforming>();
    if (existing) return false;

    auto comp = std::make_unique<components::Terraforming>();
    comp->planet_id = planet_id;
    comp->is_active = true;
    comp->stage = components::Terraforming::TerraformStage::Planning;
    entity->addComponent(std::move(comp));
    return true;
}

bool TerraformingSystem::pauseTerraforming(const std::string& entity_id) {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return false;

    tf->is_active = false;
    return true;
}

bool TerraformingSystem::resumeTerraforming(const std::string& entity_id) {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return false;

    tf->is_active = true;
    return true;
}

bool TerraformingSystem::cancelTerraforming(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* tf = entity->getComponent<components::Terraforming>();
    if (!tf) return false;

    entity->removeComponent<components::Terraforming>();
    return true;
}

bool TerraformingSystem::setTargets(const std::string& entity_id, float atmosphere,
                                     float temperature, float water_coverage) {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return false;

    tf->atmosphere_target = atmosphere;
    tf->temperature_target = temperature;
    tf->water_coverage_target = water_coverage;
    return true;
}

bool TerraformingSystem::advanceStage(const std::string& entity_id) {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return false;

    if (tf->stage == components::Terraforming::TerraformStage::Complete) return false;

    tf->stage = nextStage(tf->stage);
    tf->elapsed_in_stage = 0.0f;
    if (tf->stage == components::Terraforming::TerraformStage::Complete) {
        tf->progress = 1.0f;
        tf->total_progress = 1.0f;
    } else {
        tf->progress = 0.0f;
        int si = stageIndex(tf->stage);
        tf->total_progress = static_cast<float>(si) / 5.0f;
    }
    return true;
}

std::string TerraformingSystem::getStage(const std::string& entity_id) const {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return "unknown";

    return components::Terraforming::stageToString(tf->stage);
}

float TerraformingSystem::getProgress(const std::string& entity_id) const {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return 0.0f;

    return tf->progress;
}

float TerraformingSystem::getTotalProgress(const std::string& entity_id) const {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return 0.0f;

    return tf->total_progress;
}

bool TerraformingSystem::isActive(const std::string& entity_id) const {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return false;

    return tf->is_active;
}

double TerraformingSystem::getTotalCreditsSpent(const std::string& entity_id) const {
    auto* tf = getComponentFor(entity_id);
    if (!tf) return 0.0;

    return tf->total_credits_spent;
}

} // namespace systems
} // namespace atlas
