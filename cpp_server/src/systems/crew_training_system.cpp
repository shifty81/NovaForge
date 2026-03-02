#include "systems/crew_training_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include "components/crew_components.h"
#include <algorithm>

namespace atlas {
namespace systems {

namespace {
components::CrewTraining::TrainingSlot* findTrainee(
    components::CrewTraining* ct, const std::string& trainee_id) {
    for (auto& t : ct->trainees) {
        if (t.trainee_id == trainee_id) return &t;
    }
    return nullptr;
}
} // anonymous namespace

CrewTrainingSystem::CrewTrainingSystem(ecs::World* world)
    : System(world) {
}

void CrewTrainingSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::CrewTraining>();
    for (auto* entity : entities) {
        auto* ct = entity->getComponent<components::CrewTraining>();
        if (!ct || !ct->active) continue;

        for (auto& slot : ct->trainees) {
            if (slot.completed) continue;

            slot.progress += slot.training_rate * ct->xp_bonus * delta_time;
            if (slot.progress >= 1.0f) {
                slot.progress = 1.0f;
                slot.completed = true;
                ct->total_completed++;
            }
        }
    }
}

bool CrewTrainingSystem::initializeTraining(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto comp = std::make_unique<components::CrewTraining>();
    entity->addComponent(std::move(comp));
    return true;
}

bool CrewTrainingSystem::enrollTrainee(const std::string& entity_id,
    const std::string& trainee_id, const std::string& skill_name) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return false;
    if (static_cast<int>(ct->trainees.size()) >= ct->max_trainees) return false;
    if (findTrainee(ct, trainee_id)) return false; // duplicate

    components::CrewTraining::TrainingSlot slot;
    slot.trainee_id = trainee_id;
    slot.skill_name = skill_name;
    ct->trainees.push_back(slot);
    return true;
}

bool CrewTrainingSystem::removeTrainee(const std::string& entity_id,
    const std::string& trainee_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return false;

    auto it = std::remove_if(ct->trainees.begin(), ct->trainees.end(),
        [&](const components::CrewTraining::TrainingSlot& t) {
            return t.trainee_id == trainee_id;
        });
    if (it == ct->trainees.end()) return false;
    ct->trainees.erase(it, ct->trainees.end());
    return true;
}

int CrewTrainingSystem::getTraineeCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return 0;
    return static_cast<int>(ct->trainees.size());
}

float CrewTrainingSystem::getProgress(const std::string& entity_id,
    const std::string& trainee_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return 0.0f;
    for (const auto& t : ct->trainees) {
        if (t.trainee_id == trainee_id) return t.progress;
    }
    return 0.0f;
}

bool CrewTrainingSystem::isComplete(const std::string& entity_id,
    const std::string& trainee_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return false;
    for (const auto& t : ct->trainees) {
        if (t.trainee_id == trainee_id) return t.completed;
    }
    return false;
}

int CrewTrainingSystem::getTotalCompleted(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return 0;
    return ct->total_completed;
}

bool CrewTrainingSystem::setXpBonus(const std::string& entity_id, float bonus) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return false;
    ct->xp_bonus = bonus;
    return true;
}

float CrewTrainingSystem::getXpBonus(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return 0.0f;
    return ct->xp_bonus;
}

std::string CrewTrainingSystem::getSkillName(const std::string& entity_id,
    const std::string& trainee_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return "";
    auto* ct = entity->getComponent<components::CrewTraining>();
    if (!ct) return "";
    for (const auto& t : ct->trainees) {
        if (t.trainee_id == trainee_id) return t.skill_name;
    }
    return "";
}

} // namespace systems
} // namespace atlas
