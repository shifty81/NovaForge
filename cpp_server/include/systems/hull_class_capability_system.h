#ifndef NOVAFORGE_SYSTEMS_HULL_CLASS_CAPABILITY_SYSTEM_H
#define NOVAFORGE_SYSTEMS_HULL_CLASS_CAPABILITY_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Manages hull-class capability profiles for ships
 *
 * Maps hull classes (Frigate, Destroyer, Cruiser, Battleship, Carrier,
 * Dreadnought, Titan) to their required capability slots and internal
 * spaces. Destroyer+ ships gain hangars, rover bays, grav bike bays,
 * survival modules, and rig lockers, scaling with hull size.
 *
 * From ideas.md design:
 *   - Destroyer: 1 rover bay (S), 1 grav bike bay, 1 ship hangar (S), survival module
 *   - Cruiser:   1 rover bay (M), 1 grav bike bay, 1 ship hangar (S), survival module
 *   - Battleship: 1 rover bay (M), 1 grav bike bay (M), 1 ship hangar (M), survival module
 *   - Carrier:   2 rover bays (L), 2 grav bike bays (M), 3 ship hangars (L), survival module
 *   - Dreadnought: 2 rover bays (L), 2 grav bike bays (L), 2 ship hangars (L), survival module
 *   - Titan:     3 rover bays (XL), 3 grav bike bays (L), 4 ship hangars (L), survival module
 *   - Rig locker always attached to rover bay
 */
class HullClassCapabilitySystem : public ecs::SingleComponentSystem<components::HullClassCapabilityProfile> {
public:
    explicit HullClassCapabilitySystem(ecs::World* world);
    ~HullClassCapabilitySystem() override = default;

    std::string getName() const override { return "HullClassCapabilitySystem"; }

    // Initialization — creates and populates capability profile based on hull class
    bool initializeProfile(const std::string& entity_id, const std::string& hull_class);

    // Query: hull class info
    std::string getHullClass(const std::string& entity_id) const;

    // Query: bay counts
    int getRoverBayCount(const std::string& entity_id) const;
    int getGravBikeBayCount(const std::string& entity_id) const;
    int getShipHangarCount(const std::string& entity_id) const;

    // Query: bay size classes ("S", "M", "L", "XL" or "" if none)
    std::string getRoverBayClass(const std::string& entity_id) const;
    std::string getGravBikeBayClass(const std::string& entity_id) const;
    std::string getShipHangarClass(const std::string& entity_id) const;

    // Query: module presence
    bool hasSurvivalModule(const std::string& entity_id) const;
    bool hasRigLocker(const std::string& entity_id) const;

    // Query: power/CPU budgets
    float getMaxPowerGrid(const std::string& entity_id) const;
    float getMaxCPU(const std::string& entity_id) const;

    // Query: structural limits
    int getMaxModuleSlots(const std::string& entity_id) const;
    float getMaxCargoVolume(const std::string& entity_id) const;

    // Mutation: override individual capability values
    bool setRoverBayCount(const std::string& entity_id, int count);
    bool setGravBikeBayCount(const std::string& entity_id, int count);
    bool setShipHangarCount(const std::string& entity_id, int count);

protected:
    void updateComponent(ecs::Entity& entity, components::HullClassCapabilityProfile& cap, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_HULL_CLASS_CAPABILITY_SYSTEM_H
