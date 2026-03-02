#ifndef NOVAFORGE_SYSTEMS_FARMING_DECK_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FARMING_DECK_SYSTEM_H

#include "ecs/system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Farming deck agricultural system (Phase 14)
 *
 * Manages crop growth lifecycle with planting, watering, fertilizing,
 * and harvesting. Supports multiple crop plots with environmental controls.
 */
class FarmingDeckSystem : public ecs::System {
public:
    explicit FarmingDeckSystem(ecs::World* world);
    ~FarmingDeckSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FarmingDeckSystem"; }

    // Initialization
    bool initializeDeck(const std::string& entity_id, const std::string& owner_id, int max_plots);
    bool removeDeck(const std::string& entity_id);

    // Crop operations
    bool plantCrop(const std::string& entity_id, const std::string& plot_id,
                   components::FarmingDeck::CropType crop_type);
    float harvestCrop(const std::string& entity_id, const std::string& plot_id);
    bool waterPlot(const std::string& entity_id, const std::string& plot_id, float amount);
    bool fertilizePlot(const std::string& entity_id, const std::string& plot_id, float amount);
    bool removeCrop(const std::string& entity_id, const std::string& plot_id);

    // Query
    int getPlotCount(const std::string& entity_id) const;
    std::string getGrowthStage(const std::string& entity_id, const std::string& plot_id) const;
    float getGrowthProgress(const std::string& entity_id, const std::string& plot_id) const;
    float getTotalFoodProduced(const std::string& entity_id) const;

    // Environment
    bool setPowerEnabled(const std::string& entity_id, bool enabled);
    bool setLightLevel(const std::string& entity_id, float level);
    bool setTemperature(const std::string& entity_id, float temp);
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FARMING_DECK_SYSTEM_H
