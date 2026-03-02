#include "systems/persistence_delta_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

PersistenceDeltaSystem::PersistenceDeltaSystem(ecs::World* world)
    : System(world) {
}

void PersistenceDeltaSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::PersistenceDelta>();
    for (auto* entity : entities) {
        auto* pd = entity->getComponent<components::PersistenceDelta>();
        if (!pd || !pd->active) continue;

        // Decay non-permanent entries
        for (auto& entry : pd->entries) {
            if (entry.permanent) continue;
            if (entry.magnitude > 0.0f) {
                entry.magnitude -= entry.decay_rate * delta_time;
            } else if (entry.magnitude < 0.0f) {
                entry.magnitude += entry.decay_rate * delta_time;
            }
        }

        // Remove fully decayed entries
        pd->entries.erase(
            std::remove_if(pd->entries.begin(), pd->entries.end(),
                [](const components::PersistenceDelta::DeltaEntry& e) {
                    return !e.permanent && e.magnitude == 0.0f;
                }),
            pd->entries.end());

        // Check consequence threshold
        float total = 0.0f;
        for (const auto& entry : pd->entries) {
            total += entry.magnitude;
        }
        if (std::fabs(total) >= pd->consequence_threshold) {
            pd->consequence_triggered = true;
        }
    }
}

bool PersistenceDeltaSystem::initializeTracker(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::PersistenceDelta>();
    entity->addComponent(std::move(comp));
    return true;
}

bool PersistenceDeltaSystem::recordAction(const std::string& entity_id,
    const std::string& action_id, const std::string& category,
    float magnitude, float decay_rate, bool permanent) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return false;

    // Enforce max_entries by removing oldest
    while (static_cast<int>(pd->entries.size()) >= pd->max_entries) {
        pd->entries.erase(pd->entries.begin());
    }

    components::PersistenceDelta::DeltaEntry entry;
    entry.action_id = action_id;
    entry.category = category;
    entry.magnitude = magnitude;
    entry.timestamp = 0.0f;
    entry.decay_rate = decay_rate;
    entry.permanent = permanent;
    pd->entries.push_back(entry);

    pd->actions_recorded++;
    if (magnitude > 0.0f) {
        pd->total_positive_impact += magnitude;
    } else if (magnitude < 0.0f) {
        pd->total_negative_impact += magnitude;
    }
    return true;
}

float PersistenceDeltaSystem::getCategoryImpact(const std::string& entity_id,
    const std::string& category) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return 0.0f;
    float sum = 0.0f;
    for (const auto& entry : pd->entries) {
        if (entry.category == category) sum += entry.magnitude;
    }
    return sum;
}

float PersistenceDeltaSystem::getTotalImpact(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return 0.0f;
    float sum = 0.0f;
    for (const auto& entry : pd->entries) {
        sum += entry.magnitude;
    }
    return sum;
}

int PersistenceDeltaSystem::getActionCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return 0;
    return pd->actions_recorded;
}

bool PersistenceDeltaSystem::isConsequenceTriggered(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return false;
    return pd->consequence_triggered;
}

bool PersistenceDeltaSystem::clearConsequence(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return false;
    pd->consequence_triggered = false;
    return true;
}

int PersistenceDeltaSystem::getEntryCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return 0;
    return static_cast<int>(pd->entries.size());
}

float PersistenceDeltaSystem::getPositiveImpact(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return 0.0f;
    return pd->total_positive_impact;
}

float PersistenceDeltaSystem::getNegativeImpact(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* pd = entity->getComponent<components::PersistenceDelta>();
    if (!pd) return 0.0f;
    return pd->total_negative_impact;
}

} // namespace systems
} // namespace atlas
