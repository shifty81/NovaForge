#include "systems/farming_deck_system.h"
#include "ecs/world.h"
#include "ecs/entity.h"
#include <algorithm>
#include <cmath>

namespace atlas {
namespace systems {

FarmingDeckSystem::FarmingDeckSystem(ecs::World* world)
    : SingleComponentSystem(world) {
}

void FarmingDeckSystem::updateComponent(ecs::Entity& /*entity*/, components::FarmingDeck& deck, float delta_time) {
    if (!deck.is_powered) return;

    for (auto& plot : deck.plots) {
        if (plot.stage == components::FarmingDeck::GrowthStage::Empty ||
            plot.stage == components::FarmingDeck::GrowthStage::Harvestable ||
            plot.stage == components::FarmingDeck::GrowthStage::Withered) {
            continue;
        }

        // Consume water and nutrients
        plot.water_level -= plot.water_consumption * delta_time;
        plot.nutrient_level -= plot.nutrient_consumption * delta_time;

        if (plot.water_level <= 0.0f || plot.nutrient_level <= 0.0f) {
            plot.water_level = std::max(0.0f, plot.water_level);
            plot.nutrient_level = std::max(0.0f, plot.nutrient_level);
            plot.stage = components::FarmingDeck::GrowthStage::Withered;
            continue;
        }

        // Grow
        float temp_factor = (deck.temperature >= 18.0f && deck.temperature <= 28.0f) ? 1.0f : 0.5f;
        plot.growth_progress += plot.growth_rate * deck.light_level * temp_factor * delta_time;

        if (plot.growth_progress >= 1.0f) {
            plot.growth_progress = 1.0f;
            plot.stage = components::FarmingDeck::GrowthStage::Harvestable;
        } else if (plot.growth_progress >= 0.7f) {
            plot.stage = components::FarmingDeck::GrowthStage::Mature;
        } else if (plot.growth_progress >= 0.3f) {
            plot.stage = components::FarmingDeck::GrowthStage::Growing;
        }
    }
}

bool FarmingDeckSystem::initializeDeck(const std::string& entity_id,
                                        const std::string& owner_id,
                                        int max_plots) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* existing = entity->getComponent<components::FarmingDeck>();
    if (existing) return false;

    auto comp = std::make_unique<components::FarmingDeck>();
    comp->owner_entity_id = owner_id;
    comp->max_plots = std::max(1, max_plots);
    entity->addComponent(std::move(comp));
    return true;
}

bool FarmingDeckSystem::removeDeck(const std::string& entity_id) {
    auto* entity = world_->getEntity(entity_id);
    if (!entity) return false;

    auto* deck = entity->getComponent<components::FarmingDeck>();
    if (!deck) return false;

    entity->removeComponent<components::FarmingDeck>();
    return true;
}

bool FarmingDeckSystem::plantCrop(const std::string& entity_id,
                                   const std::string& plot_id,
                                   components::FarmingDeck::CropType crop_type) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    // Check if plot already exists
    for (auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            if (plot.stage != components::FarmingDeck::GrowthStage::Empty) return false;
            plot.crop_type = crop_type;
            plot.stage = components::FarmingDeck::GrowthStage::Planted;
            plot.growth_progress = 0.0f;
            plot.water_level = 1.0f;
            plot.nutrient_level = 1.0f;
            return true;
        }
    }

    // Add new plot if capacity allows
    if (!deck->canAddPlot()) return false;

    components::FarmingDeck::CropPlot plot;
    plot.plot_id = plot_id;
    plot.crop_type = crop_type;
    plot.stage = components::FarmingDeck::GrowthStage::Planted;
    plot.growth_progress = 0.0f;
    plot.water_level = 1.0f;
    plot.nutrient_level = 1.0f;
    deck->plots.push_back(plot);
    return true;
}

float FarmingDeckSystem::harvestCrop(const std::string& entity_id,
                                      const std::string& plot_id) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return 0.0f;

    for (auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            if (plot.stage != components::FarmingDeck::GrowthStage::Harvestable) return 0.0f;

            float yield = 10.0f * components::FarmingDeck::getYieldMultiplier(plot.crop_type);
            deck->total_food_produced += yield;

            plot.stage = components::FarmingDeck::GrowthStage::Empty;
            plot.growth_progress = 0.0f;
            plot.water_level = 1.0f;
            plot.nutrient_level = 1.0f;
            return yield;
        }
    }
    return 0.0f;
}

bool FarmingDeckSystem::waterPlot(const std::string& entity_id,
                                   const std::string& plot_id,
                                   float amount) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    for (auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            plot.water_level = std::max(0.0f, std::min(1.0f, plot.water_level + amount));
            return true;
        }
    }
    return false;
}

bool FarmingDeckSystem::fertilizePlot(const std::string& entity_id,
                                       const std::string& plot_id,
                                       float amount) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    for (auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            plot.nutrient_level = std::max(0.0f, std::min(1.0f, plot.nutrient_level + amount));
            return true;
        }
    }
    return false;
}

bool FarmingDeckSystem::removeCrop(const std::string& entity_id,
                                    const std::string& plot_id) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    for (auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            plot.stage = components::FarmingDeck::GrowthStage::Empty;
            plot.growth_progress = 0.0f;
            plot.water_level = 1.0f;
            plot.nutrient_level = 1.0f;
            return true;
        }
    }
    return false;
}

int FarmingDeckSystem::getPlotCount(const std::string& entity_id) const {
    const auto* deck = getComponentFor(entity_id);
    if (!deck) return 0;

    return deck->getPlotCount();
}

std::string FarmingDeckSystem::getGrowthStage(const std::string& entity_id,
                                               const std::string& plot_id) const {
    const auto* deck = getComponentFor(entity_id);
    if (!deck) return "unknown";

    for (const auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            return components::FarmingDeck::growthStageToString(plot.stage);
        }
    }
    return "unknown";
}

float FarmingDeckSystem::getGrowthProgress(const std::string& entity_id,
                                            const std::string& plot_id) const {
    const auto* deck = getComponentFor(entity_id);
    if (!deck) return 0.0f;

    for (const auto& plot : deck->plots) {
        if (plot.plot_id == plot_id) {
            return plot.growth_progress;
        }
    }
    return 0.0f;
}

float FarmingDeckSystem::getTotalFoodProduced(const std::string& entity_id) const {
    const auto* deck = getComponentFor(entity_id);
    if (!deck) return 0.0f;

    return deck->total_food_produced;
}

bool FarmingDeckSystem::setPowerEnabled(const std::string& entity_id, bool enabled) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    deck->is_powered = enabled;
    return true;
}

bool FarmingDeckSystem::setLightLevel(const std::string& entity_id, float level) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    deck->light_level = std::max(0.0f, std::min(1.0f, level));
    return true;
}

bool FarmingDeckSystem::setTemperature(const std::string& entity_id, float temp) {
    auto* deck = getComponentFor(entity_id);
    if (!deck) return false;

    deck->temperature = temp;
    return true;
}

} // namespace systems
} // namespace atlas
