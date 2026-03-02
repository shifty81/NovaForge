#include "systems/visual_feedback_queue_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

VisualFeedbackQueueSystem::VisualFeedbackQueueSystem(ecs::World* world)
    : System(world) {
}

void VisualFeedbackQueueSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::VisualFeedbackQueue>();
    for (auto* entity : entities) {
        auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
        if (!vfq || !vfq->active) continue;

        for (auto& effect : vfq->effects) {
            effect.lifetime -= delta_time;
            if (effect.lifetime < 0.3f && effect.lifetime > 0.0f) {
                effect.fading = true;
            }
        }

        // Remove expired effects
        auto it = vfq->effects.begin();
        while (it != vfq->effects.end()) {
            if (it->lifetime <= 0.0f) {
                vfq->total_effects_expired++;
                it = vfq->effects.erase(it);
            } else {
                ++it;
            }
        }
    }
}

bool VisualFeedbackQueueSystem::createQueue(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::VisualFeedbackQueue>();
    entity->addComponent(std::move(comp));
    return true;
}

int VisualFeedbackQueueSystem::queueEffect(const std::string& entity_id, int category,
                                            float intensity, float duration, int priority,
                                            const std::string& label) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return -1;

    // If at max, remove lowest priority effect
    if (static_cast<int>(vfq->effects.size()) >= vfq->max_effects) {
        int lowest_priority = priority;
        int lowest_idx = -1;
        for (int i = 0; i < static_cast<int>(vfq->effects.size()); i++) {
            if (vfq->effects[i].priority < lowest_priority ||
                (vfq->effects[i].priority == lowest_priority && lowest_idx == -1)) {
                lowest_priority = vfq->effects[i].priority;
                lowest_idx = i;
            }
        }
        if (lowest_idx >= 0) {
            vfq->effects.erase(vfq->effects.begin() + lowest_idx);
        } else {
            return -1;
        }
    }

    components::VisualFeedbackQueue::FeedbackEffect effect;
    effect.id = vfq->next_effect_id++;
    effect.category = category;
    effect.intensity = intensity;
    effect.lifetime = duration;
    effect.max_lifetime = duration;
    effect.priority = priority;
    effect.fading = false;
    effect.label = label;

    vfq->effects.push_back(effect);
    vfq->total_effects_queued++;
    return effect.id;
}

bool VisualFeedbackQueueSystem::removeEffect(const std::string& entity_id, int effect_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return false;

    for (auto it = vfq->effects.begin(); it != vfq->effects.end(); ++it) {
        if (it->id == effect_id) {
            vfq->effects.erase(it);
            return true;
        }
    }
    return false;
}

int VisualFeedbackQueueSystem::getActiveEffectCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return 0;
    return static_cast<int>(vfq->effects.size());
}

int VisualFeedbackQueueSystem::getEffectsByCategory(const std::string& entity_id, int category) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return 0;
    int count = 0;
    for (const auto& e : vfq->effects) {
        if (e.category == category) count++;
    }
    return count;
}

int VisualFeedbackQueueSystem::getHighestPriority(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return -1;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return -1;
    if (vfq->effects.empty()) return -1;
    int highest = vfq->effects[0].priority;
    for (const auto& e : vfq->effects) {
        if (e.priority > highest) highest = e.priority;
    }
    return highest;
}

int VisualFeedbackQueueSystem::getTotalQueued(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return 0;
    return vfq->total_effects_queued;
}

int VisualFeedbackQueueSystem::getTotalExpired(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* vfq = entity->getComponent<components::VisualFeedbackQueue>();
    if (!vfq) return 0;
    return vfq->total_effects_expired;
}

} // namespace systems
} // namespace atlas
