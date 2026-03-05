#ifndef NOVAFORGE_SYSTEMS_ANCIENT_AI_REMNANT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_ANCIENT_AI_REMNANT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Spawns AI remnant boss encounters at ancient sites
 *
 * Ancient AI remnants are autonomous boss entities guarding ancient tech sites.
 * Unlike myth-based bosses, these are site-native guardians with fixed behavior
 * patterns and difficulty based on site tier.
 */
class AncientAIRemnantSystem : public ecs::SingleComponentSystem<components::AncientAIRemnant> {
public:
    explicit AncientAIRemnantSystem(ecs::World* world);
    ~AncientAIRemnantSystem() override = default;

    std::string getName() const override { return "AncientAIRemnantSystem"; }

    /**
     * @brief Spawn an AI remnant at an ancient site
     * @param site_id Ancient site where the remnant spawns
     * @param tier Site tier (1-5), determines difficulty and remnant type
     * @return Generated remnant entity ID
     */
    std::string spawnRemnant(const std::string& site_id, int tier);

    /**
     * @brief Check if a remnant is still active
     */
    bool isRemnantActive(const std::string& remnant_id) const;

    /**
     * @brief Defeat a remnant, marking it inactive and generating rewards
     * @return true if remnant was found and defeated
     */
    bool defeatRemnant(const std::string& remnant_id);

    /**
     * @brief Get the count of active remnants
     */
    int getActiveRemnantCount() const;

    /**
     * @brief Get the difficulty rating of a remnant
     */
    float getRemnantDifficulty(const std::string& remnant_id) const;

    /**
     * @brief Get the site that spawned this remnant
     */
    std::string getRemnantSiteId(const std::string& remnant_id) const;

    /**
     * @brief Get remnant type name by index
     */
    static std::string getRemnantTypeName(int type_index);

protected:
    void updateComponent(ecs::Entity& entity, components::AncientAIRemnant& comp, float delta_time) override;

private:
    int remnant_counter_ = 0;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_ANCIENT_AI_REMNANT_SYSTEM_H
