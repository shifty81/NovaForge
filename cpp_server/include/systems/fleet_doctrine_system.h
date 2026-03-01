#ifndef NOVAFORGE_SYSTEMS_FLEET_DOCTRINE_SYSTEM_H
#define NOVAFORGE_SYSTEMS_FLEET_DOCTRINE_SYSTEM_H

#include "ecs/system.h"
#include "components/fleet_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Manages fleet doctrines — composition templates for organized fleet operations
 *
 * Defines required ship classes, roles, and minimum fleet composition.
 * Tracks readiness based on how well the current fleet matches the doctrine.
 */
class FleetDoctrineSystem : public ecs::System {
public:
    explicit FleetDoctrineSystem(ecs::World* world);
    ~FleetDoctrineSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "FleetDoctrineSystem"; }

    // Doctrine management
    bool createDoctrine(const std::string& entity_id, const std::string& doctrine_id, const std::string& name);
    bool addSlot(const std::string& entity_id, const std::string& role, const std::string& ship_class,
                 int min_count, int max_count, bool required);
    bool removeSlot(const std::string& entity_id, const std::string& role);
    bool assignShip(const std::string& entity_id, const std::string& role);
    bool unassignShip(const std::string& entity_id, const std::string& role);
    bool lockDoctrine(const std::string& entity_id, bool locked);

    // Query API
    float getReadiness(const std::string& entity_id) const;
    int getSlotCount(const std::string& entity_id) const;
    int getCurrentShipCount(const std::string& entity_id) const;
    int getTargetShipCount(const std::string& entity_id) const;
    bool isReady(const std::string& entity_id) const;
    bool isLocked(const std::string& entity_id) const;
    std::string getDoctrineName(const std::string& entity_id) const;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_FLEET_DOCTRINE_SYSTEM_H
