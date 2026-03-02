#include "systems/mission_consequence_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

MissionConsequenceSystem::MissionConsequenceSystem(ecs::World* world)
    : System(world) {
}

void MissionConsequenceSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::MissionConsequence>();
    for (auto* entity : entities) {
        auto* mc = entity->getComponent<components::MissionConsequence>();
        if (!mc) continue;

        for (auto& entry : mc->active_consequences) {
            if (entry.permanent) continue;
            entry.remaining_time -= delta_time;
            if (entry.remaining_time <= 0.0f) {
                entry.remaining_time = 0.0f;
            }
        }

        // Remove expired non-permanent consequences
        mc->active_consequences.erase(
            std::remove_if(mc->active_consequences.begin(), mc->active_consequences.end(),
                [](const components::MissionConsequence::ConsequenceEntry& e) {
                    return !e.permanent && e.remaining_time <= 0.0f;
                }),
            mc->active_consequences.end()
        );
    }
}

bool MissionConsequenceSystem::initializeConsequences(const std::string& entity_id,
                                                       const std::string& system_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::MissionConsequence>();
    if (existing) return false;

    auto comp = std::make_unique<components::MissionConsequence>();
    comp->consequence_id = entity_id;
    comp->system_id = system_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool MissionConsequenceSystem::triggerConsequence(
        const std::string& entity_id, const std::string& mission_id,
        components::MissionConsequence::ConsequenceType type,
        float magnitude, float duration,
        const std::string& target_faction, bool permanent) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mc = entity->getComponent<components::MissionConsequence>();
    if (!mc) return false;

    components::MissionConsequence::ConsequenceEntry entry;
    entry.id = "csq_" + std::to_string(mc->times_triggered);
    entry.mission_id = mission_id;
    entry.target_faction = target_faction;
    entry.type = type;
    entry.magnitude = magnitude;
    entry.remaining_time = duration;
    entry.permanent = permanent;

    mc->active_consequences.push_back(entry);
    mc->times_triggered++;

    return true;
}

int MissionConsequenceSystem::getActiveCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* mc = entity->getComponent<components::MissionConsequence>();
    if (!mc) return 0;

    return static_cast<int>(mc->active_consequences.size());
}

float MissionConsequenceSystem::getMagnitude(
        const std::string& entity_id,
        components::MissionConsequence::ConsequenceType type) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* mc = entity->getComponent<components::MissionConsequence>();
    if (!mc) return 0.0f;

    float total = 0.0f;
    for (const auto& entry : mc->active_consequences) {
        if (entry.type == type) {
            total += entry.magnitude;
        }
    }
    return total;
}

bool MissionConsequenceSystem::expireConsequence(const std::string& entity_id,
                                                  const std::string& consequence_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mc = entity->getComponent<components::MissionConsequence>();
    if (!mc) return false;

    for (auto it = mc->active_consequences.begin(); it != mc->active_consequences.end(); ++it) {
        if (it->id == consequence_id) {
            mc->active_consequences.erase(it);
            return true;
        }
    }
    return false;
}

bool MissionConsequenceSystem::isConsequenceActive(const std::string& entity_id,
                                                    const std::string& consequence_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* mc = entity->getComponent<components::MissionConsequence>();
    if (!mc) return false;

    for (const auto& entry : mc->active_consequences) {
        if (entry.id == consequence_id) return true;
    }
    return false;
}

int MissionConsequenceSystem::getPermanentCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* mc = entity->getComponent<components::MissionConsequence>();
    if (!mc) return 0;

    int count = 0;
    for (const auto& entry : mc->active_consequences) {
        if (entry.permanent) count++;
    }
    return count;
}

} // namespace systems
} // namespace atlas
