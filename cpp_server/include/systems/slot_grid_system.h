#ifndef NOVAFORGE_SYSTEMS_SLOT_GRID_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SLOT_GRID_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief 3-D slot grid for placing modules on a ship hull
 *
 * Each hull has a configurable grid of slots identified by (x,y,z) position
 * and a size category.  Modules are placed into unoccupied slots and can be
 * removed later.  The grid is tagged with a ship class and tier.
 */
class SlotGridSystem
    : public ecs::SingleComponentSystem<components::SlotGridState> {
public:
    explicit SlotGridSystem(ecs::World* world);
    ~SlotGridSystem() override = default;

    std::string getName() const override { return "SlotGridSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Slot management ---
    bool add_slot(const std::string& entity_id,
                  const std::string& slot_id,
                  int x, int y, int z, int size);
    bool remove_slot(const std::string& entity_id,
                     const std::string& slot_id);
    bool place_module(const std::string& entity_id,
                      const std::string& slot_id,
                      const std::string& module_id,
                      const std::string& module_type);
    bool remove_module(const std::string& entity_id,
                       const std::string& slot_id);
    bool clear_grid(const std::string& entity_id);

    // --- Ship metadata ---
    bool set_ship_class(const std::string& entity_id,
                        const std::string& ship_class);
    bool set_tier(const std::string& entity_id, int tier);

    // --- Queries ---
    bool        is_slot_occupied(const std::string& entity_id,
                                 const std::string& slot_id) const;
    std::string get_module_at(const std::string& entity_id,
                              const std::string& slot_id) const;
    int         get_slot_count(const std::string& entity_id) const;
    int         get_occupied_count(const std::string& entity_id) const;
    std::string get_ship_class(const std::string& entity_id) const;
    int         get_tier(const std::string& entity_id) const;
    int         get_total_modules_placed(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::SlotGridState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SLOT_GRID_SYSTEM_H
