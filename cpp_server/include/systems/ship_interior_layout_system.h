#ifndef NOVAFORGE_SYSTEMS_SHIP_INTERIOR_LAYOUT_SYSTEM_H
#define NOVAFORGE_SYSTEMS_SHIP_INTERIOR_LAYOUT_SYSTEM_H

#include "ecs/system.h"
#include "ecs/entity.h"
#include "components/game_components.h"
#include <string>
#include <vector>

namespace atlas {
namespace systems {

/**
 * @brief Procedural interior layout generator for ships
 *
 * Generates room graphs for ship interiors based on ship class.
 * Larger ships get more rooms and room types (e.g. frigates get
 * bridge + engineering + cargo, while battleships add armory,
 * medical bay, crew quarters, science lab, and hangar bay).
 */
class ShipInteriorLayoutSystem : public ecs::System {
public:
    explicit ShipInteriorLayoutSystem(ecs::World* world);
    ~ShipInteriorLayoutSystem() override = default;

    void update(float delta_time) override;
    std::string getName() const override { return "ShipInteriorLayoutSystem"; }

    // --- Generation ---

    /** Generate a full interior layout for a ship entity */
    bool generateLayout(const std::string& ship_id,
                        const std::string& ship_class);

    // --- Queries ---

    /** Get the interior layout component for a ship */
    const components::ShipInteriorLayout* getLayout(const std::string& ship_id) const;

    /** Get the number of rooms in a ship's interior */
    int getRoomCount(const std::string& ship_id) const;

    /** Find a room by type in a ship's interior */
    std::string findRoomByType(const std::string& ship_id,
                               components::ShipInteriorLayout::RoomType type) const;

    /** Check if two rooms are connected */
    bool areRoomsConnected(const std::string& ship_id,
                           const std::string& room_a,
                           const std::string& room_b) const;

    /** Get a list of rooms adjacent to a given room */
    std::vector<std::string> getAdjacentRooms(const std::string& ship_id,
                                               const std::string& room_id) const;

    // --- Helpers ---

    static std::string roomTypeName(int type);

private:
    /** Room templates by ship class */
    struct ClassTemplate {
        std::vector<components::ShipInteriorLayout::RoomType> required_rooms;
        int corridor_count = 1;
        float room_spacing = 10.0f;
    };

    ClassTemplate getClassTemplate(const std::string& ship_class) const;

    static constexpr const char* INTERIOR_PREFIX = "interior_";
};

} // namespace systems
} // namespace atlas

#endif // NOVAFORGE_SYSTEMS_SHIP_INTERIOR_LAYOUT_SYSTEM_H
