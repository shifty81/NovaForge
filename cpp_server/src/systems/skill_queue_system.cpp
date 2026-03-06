#include "systems/skill_queue_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
using SQS = components::SkillQueueState;
}

SkillQueueSystem::SkillQueueSystem(ecs::World* world)
    : SingleComponentSystem(world) {}

void SkillQueueSystem::updateComponent(ecs::Entity& entity,
    components::SkillQueueState& state, float delta_time) {
    if (!state.active || state.paused) return;
    state.elapsed_time += delta_time;

    // Advance current training
    if (!state.queue.empty()) {
        auto& current = state.queue.front();
        current.trained_time += delta_time;
        if (current.trained_time >= current.train_duration) {
            // Skill completed
            SQS::CompletedSkill completed;
            completed.skill_id = current.skill_id;
            completed.level = current.target_level;
            completed.completed_at = state.elapsed_time;
            // Check if already completed at lower level — update it
            bool found = false;
            for (auto& c : state.completed_skills) {
                if (c.skill_id == current.skill_id) {
                    c.level = current.target_level;
                    c.completed_at = state.elapsed_time;
                    found = true;
                    break;
                }
            }
            if (!found) {
                state.completed_skills.push_back(completed);
            }
            state.queue.erase(state.queue.begin());
        }
    }
}

bool SkillQueueSystem::initialize(const std::string& entity_id,
    const std::string& player_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::SkillQueueState>();
    comp->player_id = player_id;
    entity->addComponent(std::move(comp));
    return true;
}

bool SkillQueueSystem::enqueueSkill(const std::string& entity_id,
    const std::string& skill_id, int target_level, float train_duration) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    // Check duplicate in queue
    for (const auto& s : state->queue) {
        if (s.skill_id == skill_id) return false;
    }
    if (static_cast<int>(state->queue.size()) >= state->max_queue_size) return false;
    SQS::QueuedSkill qs;
    qs.skill_id = skill_id;
    qs.target_level = target_level;
    qs.train_duration = train_duration;
    state->queue.push_back(qs);
    return true;
}

bool SkillQueueSystem::dequeueSkill(const std::string& entity_id,
    const std::string& skill_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    auto it = std::find_if(state->queue.begin(), state->queue.end(),
        [&](const SQS::QueuedSkill& s) { return s.skill_id == skill_id; });
    if (it == state->queue.end()) return false;
    state->queue.erase(it);
    return true;
}

int SkillQueueSystem::getQueueLength(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->queue.size()) : 0;
}

bool SkillQueueSystem::isQueued(const std::string& entity_id,
    const std::string& skill_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& s : state->queue) {
        if (s.skill_id == skill_id) return true;
    }
    return false;
}

std::string SkillQueueSystem::getCurrentSkillId(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->queue.empty()) return "";
    return state->queue.front().skill_id;
}

float SkillQueueSystem::getCurrentProgress(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->queue.empty()) return 0.0f;
    const auto& current = state->queue.front();
    if (current.train_duration <= 0.0f) return 1.0f;
    float progress = current.trained_time / current.train_duration;
    return std::min(progress, 1.0f);
}

float SkillQueueSystem::getRemainingTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state || state->queue.empty()) return 0.0f;
    const auto& current = state->queue.front();
    float remaining = current.train_duration - current.trained_time;
    return std::max(remaining, 0.0f);
}

bool SkillQueueSystem::isTraining(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state && !state->queue.empty() && !state->paused;
}

bool SkillQueueSystem::pauseTraining(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->paused) return false;
    state->paused = true;
    return true;
}

bool SkillQueueSystem::resumeTraining(const std::string& entity_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (!state->paused) return false;
    state->paused = false;
    return true;
}

bool SkillQueueSystem::isPaused(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? state->paused : false;
}

int SkillQueueSystem::getCompletedCount(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    return state ? static_cast<int>(state->completed_skills.size()) : 0;
}

bool SkillQueueSystem::hasCompleted(const std::string& entity_id,
    const std::string& skill_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    for (const auto& c : state->completed_skills) {
        if (c.skill_id == skill_id) return true;
    }
    return false;
}

int SkillQueueSystem::getSkillLevel(const std::string& entity_id,
    const std::string& skill_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0;
    for (const auto& c : state->completed_skills) {
        if (c.skill_id == skill_id) return c.level;
    }
    return 0;
}

bool SkillQueueSystem::moveToFront(const std::string& entity_id,
    const std::string& skill_id) {
    auto* state = getComponentFor(entity_id);
    if (!state) return false;
    if (state->queue.size() < 2) return false;
    // Already at front?
    if (state->queue.front().skill_id == skill_id) return false;
    auto it = std::find_if(state->queue.begin(), state->queue.end(),
        [&](const SQS::QueuedSkill& s) { return s.skill_id == skill_id; });
    if (it == state->queue.end()) return false;
    SQS::QueuedSkill skill = *it;
    skill.trained_time = 0.0f; // Reset progress when moved
    state->queue.erase(it);
    state->queue.insert(state->queue.begin(), skill);
    return true;
}

float SkillQueueSystem::getTotalTrainingTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& s : state->queue) {
        total += s.train_duration;
    }
    return total;
}

float SkillQueueSystem::getTotalRemainingTime(const std::string& entity_id) const {
    auto* state = getComponentFor(entity_id);
    if (!state) return 0.0f;
    float total = 0.0f;
    for (const auto& s : state->queue) {
        float remaining = s.train_duration - s.trained_time;
        if (remaining > 0.0f) total += remaining;
    }
    return total;
}

} // namespace systems
} // namespace atlas
