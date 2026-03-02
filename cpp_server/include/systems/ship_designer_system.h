#ifndef NOVAFORGE_SYSTEMS_SHIP_DESIGNER_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_DESIGNER_SYSTEM_H

#include "ecs/system.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship fitting designer with CPU/powergrid budget validation
 *
 * Manages ship blueprints and module fitting, tracking resource budgets
 * (CPU, powergrid) and slot availability across high/mid/low/rig slots.
 */
class ShipDesignerSystem : public ecs::System {
public:
    explicit ShipDesignerSystem(ecs::World* world);
    ~ShipDesignerSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ShipDesignerSystem"; }

    bool createDesigner(const std::string& entity_id);
    bool setBlueprint(const std::string& entity_id, const std::string& name,
                      const std::string& hull_type, const std::string& faction);
    bool fitModule(const std::string& entity_id, const std::string& module_name,
                   int slot_type, float cpu_cost, float power_cost);
    bool removeModule(const std::string& entity_id, const std::string& module_name);
    bool validateDesign(const std::string& entity_id);
    float getCpuUsage(const std::string& entity_id) const;
    float getPowerUsage(const std::string& entity_id) const;
    int getFittedCount(const std::string& entity_id) const;
    int getSlotsFree(const std::string& entity_id, int slot_type) const;
    bool clearDesign(const std::string& entity_id);
    bool isValid(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_DESIGNER_SYSTEM_H
