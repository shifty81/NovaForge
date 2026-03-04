#ifndef NOVAFORGE_SYSTEMS_PLANETARY_TRAVERSAL_SYSTEM_H
#define NOVAFORGE_SYSTEMS_PLANETARY_TRAVERSAL_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/exploration_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Planetary surface traversal system (Phase 14)
 *
 * Manages surface movement across planetary terrain with vehicle support,
 * terrain speed modifiers, and waypoint-based navigation.
 */
class PlanetaryTraversalSystem : public ecs::SingleComponentSystem<components::PlanetaryTraversal> {
public:
    explicit PlanetaryTraversalSystem(ecs::World* world);
    ~PlanetaryTraversalSystem() override = default;

    std::string getName() const override { return "PlanetaryTraversalSystem"; }

    // Initialization
    bool initializeTraversal(const std::string& entity_id, const std::string& planet_id,
                             float start_x, float start_y);
    bool removeTraversal(const std::string& entity_id);

    // Navigation
    bool setDestination(const std::string& entity_id, float dest_x, float dest_y);
    bool clearDestination(const std::string& entity_id);

    // Vehicle
    bool setVehicle(const std::string& entity_id, const std::string& vehicle_id, float max_speed);
    bool dismountVehicle(const std::string& entity_id);

    // Terrain
    bool setTerrainType(const std::string& entity_id, components::PlanetaryTraversal::TerrainType type);

    // Query
    float getPositionX(const std::string& entity_id) const;
    float getPositionY(const std::string& entity_id) const;
    float getSpeed(const std::string& entity_id) const;
    float getDistanceTraveled(const std::string& entity_id) const;
    bool isTraversing(const std::string& entity_id) const;
    float getDistanceToDestination(const std::string& entity_id) const;
    std::string getTerrainTypeStr(const std::string& entity_id) const;

protected:
    void updateComponent(ecs::Entity& entity, components::PlanetaryTraversal& trav, float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_PLANETARY_TRAVERSAL_SYSTEM_H
