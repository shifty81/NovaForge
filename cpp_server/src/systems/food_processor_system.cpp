#include "systems/food_processor_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>

namespace atlas {
namespace systems {

FoodProcessorSystem::FoodProcessorSystem(ecs::World* world)
    : System(world) {
}

void FoodProcessorSystem::update(float delta_time) {
    auto entities = world_->getEntities<components::FoodProcessor>();
    for (auto* entity : entities) {
        auto* fp = entity->getComponent<components::FoodProcessor>();
        if (!fp || !fp->powered) continue;

        for (auto& job : fp->active_jobs) {
            if (job.completed) continue;
            job.time_remaining -= delta_time * fp->efficiency;
            if (job.time_remaining <= 0.0f) {
                job.time_remaining = 0.0f;
                job.completed = true;
            }
        }
    }
}

bool FoodProcessorSystem::addRecipe(const std::string& entity_id, const std::string& recipe_id,
                                     const std::string& output_item, int output_quantity,
                                     const std::vector<std::pair<std::string, int>>& ingredients,
                                     float craft_time, float nutrition_value) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    // Check for duplicate recipe
    for (const auto& r : fp->available_recipes) {
        if (r.recipe_id == recipe_id) return false;
    }

    components::FoodProcessor::Recipe recipe;
    recipe.recipe_id = recipe_id;
    recipe.output_item = output_item;
    recipe.output_quantity = output_quantity;
    recipe.ingredients = ingredients;
    recipe.craft_time = craft_time;
    recipe.nutrition_value = nutrition_value;
    fp->available_recipes.push_back(recipe);
    return true;
}

bool FoodProcessorSystem::startCrafting(const std::string& entity_id, const std::string& recipe_id,
                                         const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    if (!fp->powered) return false;

    // Check max concurrent jobs
    int active_count = 0;
    for (const auto& j : fp->active_jobs) {
        if (!j.completed) active_count++;
    }
    if (active_count >= fp->max_concurrent_jobs) return false;

    // Find recipe
    const components::FoodProcessor::Recipe* found = nullptr;
    for (const auto& r : fp->available_recipes) {
        if (r.recipe_id == recipe_id) {
            found = &r;
            break;
        }
    }
    if (!found) return false;

    components::FoodProcessor::CraftJob job;
    job.recipe_id = recipe_id;
    job.time_remaining = found->craft_time;
    job.total_time = found->craft_time;
    job.completed = false;
    job.owner_id = owner_id;
    fp->active_jobs.push_back(job);
    return true;
}

bool FoodProcessorSystem::cancelCrafting(const std::string& entity_id, const std::string& owner_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    for (auto it = fp->active_jobs.begin(); it != fp->active_jobs.end(); ++it) {
        if (it->owner_id == owner_id && !it->completed) {
            fp->active_jobs.erase(it);
            return true;
        }
    }
    return false;
}

bool FoodProcessorSystem::setPowered(const std::string& entity_id, bool powered) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    fp->powered = powered;
    return true;
}

bool FoodProcessorSystem::setEfficiency(const std::string& entity_id, float efficiency) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    fp->efficiency = efficiency;
    return true;
}

int FoodProcessorSystem::getRecipeCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return 0;

    return static_cast<int>(fp->available_recipes.size());
}

int FoodProcessorSystem::getActiveJobCount(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return 0;

    int count = 0;
    for (const auto& j : fp->active_jobs) {
        if (!j.completed) count++;
    }
    return count;
}

bool FoodProcessorSystem::isJobComplete(const std::string& entity_id, const std::string& owner_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    for (const auto& j : fp->active_jobs) {
        if (j.owner_id == owner_id && j.completed) return true;
    }
    return false;
}

bool FoodProcessorSystem::isPowered(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return false;

    return fp->powered;
}

float FoodProcessorSystem::getEfficiency(const std::string& entity_id) const {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return 0.0f;

    auto* fp = entity->getComponent<components::FoodProcessor>();
    if (!fp) return 0.0f;

    return fp->efficiency;
}

} // namespace systems
} // namespace atlas
