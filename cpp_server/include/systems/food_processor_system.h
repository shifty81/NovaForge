#ifndef NOVAFORGE_SYSTEMS_FOOD_PROCESSOR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FOOD_PROCESSOR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/fps_components.h"
#include <string>
#include <vector>
#include <utility>

namespace atlas {
namespace systems {

/**
 * @brief Survival module food processing system (Phase 13)
 *
 * Manages food processors that craft nutrition items from ingredient recipes.
 * Supports concurrent crafting jobs, power toggling, and efficiency multipliers.
 */
class FoodProcessorSystem : public ecs::SingleComponentSystem<components::FoodProcessor> {
public:
    explicit FoodProcessorSystem(ecs::World* world);
    ~FoodProcessorSystem() override = default;

    std::string getName() const override { return "FoodProcessorSystem"; }

    // Commands
    bool addRecipe(const std::string& entity_id, const std::string& recipe_id,
                   const std::string& output_item, int output_quantity,
                   const std::vector<std::pair<std::string, int>>& ingredients,
                   float craft_time, float nutrition_value);
    bool startCrafting(const std::string& entity_id, const std::string& recipe_id,
                       const std::string& owner_id);
    bool cancelCrafting(const std::string& entity_id, const std::string& owner_id);
    bool setPowered(const std::string& entity_id, bool powered);
    bool setEfficiency(const std::string& entity_id, float efficiency);

    // Query API
    int getRecipeCount(const std::string& entity_id) const;
    int getActiveJobCount(const std::string& entity_id) const;
    bool isJobComplete(const std::string& entity_id, const std::string& owner_id) const;
    bool isPowered(const std::string& entity_id) const;
    float getEfficiency(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::FoodProcessor& fp, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FOOD_PROCESSOR_SYSTEM_H
