#include "systems/rest_station_system.h"
#include "ecs/world.h"
#include <algorithm>
#include <memory>

namespace atlas {
namespace systems {

RestStationSystem::RestStationSystem(ecs::World* world)
    : System(world) {
}

void RestStationSystem::update(float delta_time) {
    // Process all entities that are currently resting
    auto entities = world_->getEntities<components::RestingState>();
    for (auto* entity : entities) {
        auto* resting = entity->getComponent<components::RestingState>();
        if (!resting || !resting->is_resting) continue;

        auto* needs = entity->getComponent<components::SurvivalNeeds>();
        if (!needs) continue;

        // Get the rest station for quality multiplier
        auto* station_entity = world_->getEntity(resting->rest_station_id);
        float quality = 1.0f;
        if (station_entity) {
            auto* station = station_entity->getComponent<components::RestStation>();
            if (station) {
                quality = station->quality;
                station->total_rest_time += delta_time;
            }
        }

        // Recover fatigue based on rest quality
        float recovery = base_recovery_rate_ * quality * delta_time;
        needs->fatigue = std::max(0.0f, needs->fatigue - recovery);

        // Stop resting automatically when fully rested
        if (needs->fatigue <= 0.0f) {
            stopResting(entity->getId());
        }
    }
}

bool RestStationSystem::startResting(const std::string& entity_id,
                                      const std::string& station_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* needs = entity->getComponent<components::SurvivalNeeds>();
    if (!needs) return false;

    auto* station_entity = world_->getEntity(station_id);
    if (!station_entity) return false;

    auto* station = station_entity->getComponent<components::RestStation>();
    if (!station) return false;

    // Check if station is available
    if (!station->isAvailable()) return false;

    // Mark station as occupied
    station->startRest(entity_id, 0.0f);  // timestamp handled externally if needed

    // Add or update resting state component
    auto* resting = entity->getComponent<components::RestingState>();
    if (!resting) {
        auto restState = std::make_unique<components::RestingState>();
        resting = restState.get();
        entity->addComponent(std::move(restState));
    }

    resting->rest_station_id = station_id;
    resting->rest_start_time = 0.0f;
    resting->fatigue_at_start = needs->fatigue;
    resting->is_resting = true;

    return true;
}

float RestStationSystem::stopResting(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 100.0f;

    auto* resting = entity->getComponent<components::RestingState>();
    if (!resting || !resting->is_resting) {
        auto* needs = entity->getComponent<components::SurvivalNeeds>();
        return needs ? needs->fatigue : 100.0f;
    }

    // Free up the station
    auto* station_entity = world_->getEntity(resting->rest_station_id);
    if (station_entity) {
        auto* station = station_entity->getComponent<components::RestStation>();
        if (station) {
            station->endRest();
        }
    }

    // Clear resting state
    resting->is_resting = false;
    resting->rest_station_id.clear();

    auto* needs = entity->getComponent<components::SurvivalNeeds>();
    return needs ? needs->fatigue : 0.0f;
}

bool RestStationSystem::isResting(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* resting = entity->getComponent<components::RestingState>();
    return resting && resting->is_resting;
}

float RestStationSystem::getRestProgress(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* resting = entity->getComponent<components::RestingState>();
    if (!resting || !resting->is_resting) return 0.0f;

    auto* needs = entity->getComponent<components::SurvivalNeeds>();
    if (!needs || resting->fatigue_at_start <= 0.0f) return 1.0f;

    float recovered = resting->fatigue_at_start - needs->fatigue;
    return std::min(1.0f, recovered / resting->fatigue_at_start);
}

float RestStationSystem::getTimeUntilFullyRested(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* resting = entity->getComponent<components::RestingState>();
    if (!resting || !resting->is_resting) return 0.0f;

    auto* needs = entity->getComponent<components::SurvivalNeeds>();
    if (!needs || needs->fatigue <= 0.0f) return 0.0f;

    // Get station quality
    float quality = 1.0f;
    auto* station_entity = world_->getEntity(resting->rest_station_id);
    if (station_entity) {
        auto* station = station_entity->getComponent<components::RestStation>();
        if (station) {
            quality = station->quality;
        }
    }

    float effective_rate = base_recovery_rate_ * quality;
    if (effective_rate <= 0.0f) return 0.0f;

    return needs->fatigue / effective_rate;
}

bool RestStationSystem::isStationAvailable(const std::string& station_id) const {
    auto* entity = world_->getEntity(station_id);
    if (!entity) return false;

    auto* station = entity->getComponent<components::RestStation>();
    return station && station->isAvailable();
}

float RestStationSystem::getStationQuality(const std::string& station_id) const {
    auto* entity = world_->getEntity(station_id);
    if (!entity) return 1.0f;

    auto* station = entity->getComponent<components::RestStation>();
    return station ? station->quality : 1.0f;
}

std::string RestStationSystem::getQualityName(float quality) {
    if (quality >= 2.0f) return "Luxury";
    if (quality >= 1.5f) return "Premium";
    if (quality >= 1.0f) return "Standard";
    if (quality >= 0.5f) return "Basic";
    return "Poor";
}

int RestStationSystem::getRestingCount() const {
    int count = 0;
    auto entities = world_->getEntities<components::RestingState>();
    for (auto* entity : entities) {
        auto* resting = entity->getComponent<components::RestingState>();
        if (resting && resting->is_resting) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
