#ifndef NOVAFORGE_SYSTEMS_CONSTRUCTION_PLACEMENT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_CONSTRUCTION_PLACEMENT_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/game_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * ConstructionPlacementSystem - socket-based grid module placement
 * for ship/station building.
 *
 * Reads/Writes ConstructionPlacementState component.
 *
 * Design:
 *   - Snaps placement coordinates to a configurable grid.
 *   - Tracks occupied vs. available sockets per hull.
 *   - Supports multiple build contexts (ShipInterior, ShipExterior, etc.).
 */
class ConstructionPlacementSystem : public ecs::SingleComponentSystem<components::ConstructionPlacementState> {
public:
    explicit ConstructionPlacementSystem(ecs::World* world);
    ~ConstructionPlacementSystem() override = default;

    std::string getName() const override { return "ConstructionPlacementSystem"; }

    /// Place a module at the given coordinates (snapped to grid). Returns false if sockets full.
    bool placeModule(const std::string& entity_id, const std::string& module_id, float x, float y, float z);

    /// Remove a module from a socket index. Returns false if index invalid or empty.
    bool removeModule(const std::string& entity_id, int socket_index);

    /// Change build context.
    bool setContext(const std::string& entity_id, components::ConstructionPlacementState::BuildContext ctx);

    /// Select a module for placement preview.
    bool selectModule(const std::string& entity_id, const std::string& module_id);

    /// Validate whether a position is grid-aligned and a socket is available.
    bool validatePlacement(const std::string& entity_id, float x, float y, float z) const;

    /// Query helpers
    int getOccupiedSockets(const std::string& entity_id) const;
    int getAvailableSockets(const std::string& entity_id) const;

    /// Static grid helpers
    static bool isGridAligned(float x, float y, float z, float grid_size);
    static float snapToGrid(float value, float grid_size);

protected:
    void updateComponent(ecs::Entity& entity, components::ConstructionPlacementState& state, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_CONSTRUCTION_PLACEMENT_SYSTEM_H
