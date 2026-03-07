#ifndef NOVAFORGE_SYSTEMS_CARGO_HOLD_MANAGEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CARGO_HOLD_MANAGEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Cargo hold capacity management — add, remove, stack, jettison items
 *
 * Manages a ship's cargo hold with per-item volume tracking.  Items are
 * stacked by item_id.  Adding items that exceed remaining capacity is
 * rejected.  Items can be jettisoned (removed and flagged as floating).
 * Each tick updates elapsed time for time-sensitive cargo (e.g. perishables).
 */
class CargoHoldManagementSystem : public ecs::SingleComponentSystem<components::CargoHoldState> {
public:
    explicit CargoHoldManagementSystem(ecs::World* world);
    ~CargoHoldManagementSystem() override = default;

    std::string getName() const override { return "CargoHoldManagementSystem"; }

public:
    bool initialize(const std::string& entity_id, float max_volume);
    bool addItem(const std::string& entity_id, const std::string& item_id,
                 int quantity, float volume_per_unit);
    bool removeItem(const std::string& entity_id, const std::string& item_id,
                    int quantity);
    bool jettisonItem(const std::string& entity_id, const std::string& item_id);
    bool setMaxVolume(const std::string& entity_id, float max_volume);

    int getItemCount(const std::string& entity_id) const;
    int getItemQuantity(const std::string& entity_id, const std::string& item_id) const;
    float getUsedVolume(const std::string& entity_id) const;
    float getFreeVolume(const std::string& entity_id) const;
    float getMaxVolume(const std::string& entity_id) const;
    int getTotalJettisoned(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::CargoHoldState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CARGO_HOLD_MANAGEMENT_SYSTEM_H
