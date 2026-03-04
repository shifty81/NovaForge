#ifndef NOVAFORGE_SYSTEMS_VISUAL_RIG_SYSTEM_H
#define NOVAFORGE_SYSTEMS_VISUAL_RIG_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Visual rig generation system (Phase 13)
 *
 * Manages visual state of rigs based on installed modules. Computes visual
 * properties like thruster configuration, cargo size, and attached items
 * from the RigLoadout component.
 */
class VisualRigSystem : public ecs::SingleComponentSystem<components::VisualRigState> {
public:
    explicit VisualRigSystem(ecs::World* world);
    ~VisualRigSystem() override = default;

    std::string getName() const override { return "VisualRigSystem"; }

    // Visual state initialization
    bool initializeVisualState(const std::string& entity_id, uint64_t visual_seed);
    bool removeVisualState(const std::string& entity_id);

    // Update visual state from rig loadout
    bool updateFromLoadout(const std::string& entity_id);

    // Thruster configuration
    std::string getThrusterConfig(const std::string& entity_id) const;
    bool setThrusterScale(const std::string& entity_id, float scale);
    float getThrusterScale(const std::string& entity_id) const;

    // Cargo configuration
    std::string getCargoSize(const std::string& entity_id) const;
    bool setCargoScale(const std::string& entity_id, float scale);
    float getCargoScale(const std::string& entity_id) const;

    // Visual features
    bool hasShieldEmitter(const std::string& entity_id) const;
    bool hasAntenna(const std::string& entity_id) const;
    bool hasSolarPanels(const std::string& entity_id) const;
    bool hasDroneBay(const std::string& entity_id) const;
    int getWeaponMountCount(const std::string& entity_id) const;
    int getToolMountCount(const std::string& entity_id) const;

    // Bulk and glow
    float getTotalBulk(const std::string& entity_id) const;
    float getGlowIntensity(const std::string& entity_id) const;
    bool setGlowIntensity(const std::string& entity_id, float intensity);

    // Colors
    bool setColors(const std::string& entity_id, const std::string& primary,
                   const std::string& secondary);
    std::string getPrimaryColor(const std::string& entity_id) const;
    std::string getSecondaryColor(const std::string& entity_id) const;

    // Trinkets
    bool addTrinket(const std::string& entity_id, const std::string& trinket_id);
    bool removeTrinket(const std::string& entity_id, const std::string& trinket_id);
    int getTrinketCount(const std::string& entity_id) const;
    bool canAddTrinket(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::VisualRigState& visual, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_VISUAL_RIG_SYSTEM_H
