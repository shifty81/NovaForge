#include "systems/ship_interior_layout_system.h"
#include "ecs/world.h"
#include <cmath>

namespace atlas {
namespace systems {

ShipInteriorLayoutSystem::ShipInteriorLayoutSystem(ecs::World* world)
    : System(world) {
}

void ShipInteriorLayoutSystem::update(float /*delta_time*/) {
    // Layout generation is event-driven, no per-tick work.
}

// ---------------------------------------------------------------------------
// Generation
// ---------------------------------------------------------------------------

bool ShipInteriorLayoutSystem::generateLayout(
        const std::string& ship_id,
        const std::string& ship_class) {

    std::string interior_eid = std::string(INTERIOR_PREFIX) + ship_id;
    if (world_->getEntity(interior_eid)) return false; // already generated

    auto* entity = world_->createEntity(interior_eid);
    if (!entity) return false;

    auto layout = std::make_unique<components::ShipInteriorLayout>();
    layout->interior_id = interior_eid;
    layout->ship_id     = ship_id;
    layout->ship_class  = ship_class;

    ClassTemplate tmpl = getClassTemplate(ship_class);

    float z_offset = 0.0f;

    // Generate required rooms
    for (size_t i = 0; i < tmpl.required_rooms.size(); ++i) {
        components::ShipInteriorLayout::Room room;
        room.room_id   = interior_eid + "_room_" + std::to_string(i);
        room.room_type = static_cast<int>(tmpl.required_rooms[i]);
        room.pos_z     = z_offset;

        // Scale room size by type
        using RT = components::ShipInteriorLayout::RoomType;
        auto rt = tmpl.required_rooms[i];
        if (rt == RT::Bridge)       { room.size_x = 8.0f;  room.size_z = 6.0f;  }
        else if (rt == RT::Engineering)  { room.size_x = 10.0f; room.size_z = 8.0f;  }
        else if (rt == RT::CargoHold)    { room.size_x = 12.0f; room.size_z = 10.0f; }
        else if (rt == RT::HangarBay)    { room.size_x = 20.0f; room.size_z = 15.0f; room.size_y = 6.0f; }
        else if (rt == RT::Corridor)     { room.size_x = 3.0f;  room.size_z = 8.0f;  }
        else                             { room.size_x = 6.0f;  room.size_z = 6.0f;  }

        layout->rooms.push_back(room);
        z_offset += tmpl.room_spacing;
    }

    // Generate connections between sequential rooms
    for (size_t i = 0; i + 1 < layout->rooms.size(); ++i) {
        components::ShipInteriorLayout::Connection conn;
        conn.from_room_id = layout->rooms[i].room_id;
        conn.to_room_id   = layout->rooms[i + 1].room_id;
        conn.door_id      = interior_eid + "_door_" + std::to_string(i);
        layout->connections.push_back(conn);
    }

    // Airlock rooms get an external connection to "exterior"
    int airlock_idx = 0;
    for (const auto& room : layout->rooms) {
        using RT = components::ShipInteriorLayout::RoomType;
        if (room.room_type == static_cast<int>(RT::Airlock)) {
            components::ShipInteriorLayout::Connection conn;
            conn.from_room_id = room.room_id;
            conn.to_room_id   = "exterior";
            conn.door_id      = interior_eid + "_airlock_door_" + std::to_string(airlock_idx);
            layout->connections.push_back(conn);
            ++airlock_idx;
        }
    }

    entity->addComponent(std::move(layout));
    return true;
}

// ---------------------------------------------------------------------------
// Queries
// ---------------------------------------------------------------------------

const components::ShipInteriorLayout* ShipInteriorLayoutSystem::getLayout(
        const std::string& ship_id) const {
    std::string interior_eid = std::string(INTERIOR_PREFIX) + ship_id;
    auto* entity = world_->getEntity(interior_eid);
    if (!entity) return nullptr;
    return entity->getComponent<components::ShipInteriorLayout>();
}

int ShipInteriorLayoutSystem::getRoomCount(const std::string& ship_id) const {
    auto* layout = getLayout(ship_id);
    return layout ? layout->roomCount() : 0;
}

std::string ShipInteriorLayoutSystem::findRoomByType(
        const std::string& ship_id,
        components::ShipInteriorLayout::RoomType type) const {
    auto* layout = getLayout(ship_id);
    if (!layout) return "";
    int t = static_cast<int>(type);
    for (const auto& room : layout->rooms) {
        if (room.room_type == t) return room.room_id;
    }
    return "";
}

bool ShipInteriorLayoutSystem::areRoomsConnected(
        const std::string& ship_id,
        const std::string& room_a,
        const std::string& room_b) const {
    auto* layout = getLayout(ship_id);
    if (!layout) return false;
    return layout->areConnected(room_a, room_b);
}

std::vector<std::string> ShipInteriorLayoutSystem::getAdjacentRooms(
        const std::string& ship_id,
        const std::string& room_id) const {
    std::vector<std::string> result;
    auto* layout = getLayout(ship_id);
    if (!layout) return result;
    for (const auto& c : layout->connections) {
        if (c.from_room_id == room_id) result.push_back(c.to_room_id);
        else if (c.to_room_id == room_id) result.push_back(c.from_room_id);
    }
    return result;
}

// ---------------------------------------------------------------------------
// Helpers
// ---------------------------------------------------------------------------

std::string ShipInteriorLayoutSystem::roomTypeName(int type) {
    return components::ShipInteriorLayout::roomTypeName(type);
}

ShipInteriorLayoutSystem::ClassTemplate
ShipInteriorLayoutSystem::getClassTemplate(const std::string& ship_class) const {
    using RT = components::ShipInteriorLayout::RoomType;
    ClassTemplate tmpl;

    if (ship_class == "frigate" || ship_class == "destroyer") {
        tmpl.required_rooms = { RT::Bridge, RT::Corridor, RT::Engineering, RT::CargoHold, RT::Airlock };
        tmpl.corridor_count = 1;
        tmpl.room_spacing   = 8.0f;
    } else if (ship_class == "cruiser" || ship_class == "battlecruiser") {
        tmpl.required_rooms = { RT::Bridge, RT::Corridor, RT::CrewQuarters,
                                RT::Engineering, RT::Corridor, RT::CargoHold,
                                RT::MedicalBay, RT::Airlock };
        tmpl.corridor_count = 1;
        tmpl.room_spacing   = 10.0f;
    } else if (ship_class == "battleship") {
        tmpl.required_rooms = { RT::Bridge, RT::Corridor, RT::CrewQuarters,
                                RT::Armory, RT::Corridor, RT::Engineering,
                                RT::MedicalBay, RT::ScienceLab, RT::CargoHold,
                                RT::Airlock };
        tmpl.corridor_count = 1;
        tmpl.room_spacing   = 12.0f;
    } else if (ship_class == "capital" || ship_class == "carrier" ||
               ship_class == "dreadnought" || ship_class == "titan") {
        tmpl.required_rooms = { RT::Bridge, RT::Corridor, RT::CrewQuarters,
                                RT::Armory, RT::Corridor, RT::Engineering,
                                RT::MedicalBay, RT::ScienceLab, RT::CargoHold,
                                RT::HangarBay, RT::Corridor, RT::Airlock };
        tmpl.corridor_count = 1;
        tmpl.room_spacing   = 15.0f;
    } else {
        // Default / unknown class — minimal layout
        tmpl.required_rooms = { RT::Bridge, RT::Corridor, RT::Engineering, RT::Airlock };
        tmpl.corridor_count = 1;
        tmpl.room_spacing   = 8.0f;
    }

    return tmpl;
}

} // namespace systems
} // namespace atlas
