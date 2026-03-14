#ifndef NOVAFORGE_SYSTEMS_HANGAR_SYSTEM_H
#define NOVAFORGE_SYSTEMS_HANGAR_SYSTEM_H

#include "ecs/single_component_system.h"
#include "components/ship_components.h"
#include <string>

namespace atlas {
namespace systems {

/**
 * @brief Ship hangar management system
 *
 * Manages a player's stored ships at a station.  Each hangar has a capacity
 * cap (max_ships, default 50).  Ships can be stored, retrieved, renamed,
 * set as the active ship (only one active at a time), and insured.
 * Lifetime counters track total_ships_stored and total_ships_retrieved.
 */
class HangarSystem
    : public ecs::SingleComponentSystem<components::HangarState> {
public:
    explicit HangarSystem(ecs::World* world);
    ~HangarSystem() override = default;

    std::string getName() const override { return "HangarSystem"; }

    // --- Lifecycle ---
    bool initialize(const std::string& entity_id);

    // --- Ship management ---
    bool storeShip(const std::string& entity_id,
                   const std::string& ship_id,
                   const std::string& ship_type,
                   const std::string& ship_name);
    bool retrieveShip(const std::string& entity_id,
                      const std::string& ship_id);
    bool renameShip(const std::string& entity_id,
                    const std::string& ship_id,
                    const std::string& new_name);
    bool setActiveShip(const std::string& entity_id,
                       const std::string& ship_id);
    bool setInsurance(const std::string& entity_id,
                      const std::string& ship_id,
                      float insurance_value);
    bool clearHangar(const std::string& entity_id);

    // --- Configuration ---
    bool setStationId(const std::string& entity_id,
                      const std::string& station_id);
    bool setMaxShips(const std::string& entity_id, int max_ships);

    // --- Queries ---
    int  getShipCount(const std::string& entity_id) const;
    bool hasShip(const std::string& entity_id,
                 const std::string& ship_id) const;
    std::string getShipName(const std::string& entity_id,
                            const std::string& ship_id) const;
    std::string getShipType(const std::string& entity_id,
                            const std::string& ship_id) const;
    std::string getActiveShipId(const std::string& entity_id) const;
    std::string getStationId(const std::string& entity_id) const;
    float getInsurance(const std::string& entity_id,
                       const std::string& ship_id) const;
    int  getTotalShipsStored(const std::string& entity_id) const;
    int  getTotalShipsRetrieved(const std::string& entity_id) const;
    int  getShipCountByType(const std::string& entity_id,
                            const std::string& ship_type) const;

protected:
    void updateComponent(ecs::Entity& entity,
                         components::HangarState& comp,
                         float delta_time) override;
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_HANGAR_SYSTEM_H
