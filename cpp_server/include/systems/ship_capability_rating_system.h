#ifndef NOVAFORGE_SYSTEMS_SHIP_CAPABILITY_RATING_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_CAPABILITY_RATING_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Computes per-ship 0-5 star capability ratings
 *
 * Evaluates ship loadout to produce ratings across 6 categories:
 * Combat, Mining, Exploration, Cargo, Defense, Fabrication.
 * Rating is 0-5 stars based on module counts and stats.
 */
class ShipCapabilityRatingSystem : public ecs::SingleComponentSystem<components::ShipCapabilityRating> {
public:
    explicit ShipCapabilityRatingSystem(ecs::World* world);
    ~ShipCapabilityRatingSystem() override = default;

    std::string getName() const override { return "ShipCapabilityRatingSystem"; }

    bool initializeRating(const std::string& entity_id);
    bool setWeaponCount(const std::string& entity_id, int count);
    bool setMiningModuleCount(const std::string& entity_id, int count);
    bool setScannerCount(const std::string& entity_id, int count);
    bool setCargoCapacity(const std::string& entity_id, float capacity_m3);
    bool setTotalEHP(const std::string& entity_id, float ehp);
    bool setIndustryModuleCount(const std::string& entity_id, int count);
    bool recalculate(const std::string& entity_id);
    float getCombatRating(const std::string& entity_id) const;
    float getMiningRating(const std::string& entity_id) const;
    float getExplorationRating(const std::string& entity_id) const;
    float getCargoRating(const std::string& entity_id) const;
    float getDefenseRating(const std::string& entity_id) const;
    float getFabricationRating(const std::string& entity_id) const;
    float getOverallRating(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShipCapabilityRating& scr, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_CAPABILITY_RATING_SYSTEM_H
