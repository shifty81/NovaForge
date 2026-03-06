#ifndef NOVAFORGE_SYSTEMS_SHIP_FITTING_VALIDATION_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_FITTING_VALIDATION_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Validates ship module fittings against CPU, power grid, and slot constraints
 *
 * When modules are fitted or removed, this system recalculates total CPU and
 * power grid usage, validates slot type compatibility (high/mid/low), checks
 * capacity limits, and flags over-fit states. Provides real-time fitting stats
 * for the ship fitting UI and prevents undocking with invalid configurations.
 */
class ShipFittingValidationSystem : public ecs::SingleComponentSystem<components::ShipFittingValidationState> {
public:
    explicit ShipFittingValidationSystem(ecs::World* world);
    ~ShipFittingValidationSystem() override = default;

    std::string getName() const override { return "ShipFittingValidationSystem"; }

public:
    bool initialize(const std::string& entity_id, float max_cpu, float max_power_grid,
                    int high_slots, int mid_slots, int low_slots);

    // Module fitting
    bool fitModule(const std::string& entity_id, const std::string& module_id,
                   const std::string& slot_type, float cpu_usage, float power_grid_usage);
    bool unfitModule(const std::string& entity_id, const std::string& module_id);
    bool hasFittedModule(const std::string& entity_id, const std::string& module_id) const;
    int getFittedModuleCount(const std::string& entity_id) const;

    // Resource queries
    float getCpuUsed(const std::string& entity_id) const;
    float getCpuRemaining(const std::string& entity_id) const;
    float getPowerGridUsed(const std::string& entity_id) const;
    float getPowerGridRemaining(const std::string& entity_id) const;

    // Slot queries
    int getHighSlotsUsed(const std::string& entity_id) const;
    int getMidSlotsUsed(const std::string& entity_id) const;
    int getLowSlotsUsed(const std::string& entity_id) const;
    int getHighSlotsRemaining(const std::string& entity_id) const;
    int getMidSlotsRemaining(const std::string& entity_id) const;
    int getLowSlotsRemaining(const std::string& entity_id) const;

    // Validation
    bool isValidFit(const std::string& entity_id) const;
    bool isCpuOverloaded(const std::string& entity_id) const;
    bool isPowerGridOverloaded(const std::string& entity_id) const;
    int getValidationErrorCount(const std::string& entity_id) const;

    // Summary
    float getCpuUtilization(const std::string& entity_id) const;
    float getPowerGridUtilization(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::ShipFittingValidationState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_FITTING_VALIDATION_SYSTEM_H
