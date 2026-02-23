#include "pcg/interior_budget.h"

namespace atlas {
namespace pcg {

// ── Scaling table (newupload.txt §3) ────────────────────────────────
//
// Ship Class    Rooms     Corridors  Avg Room  CorrWidth   Levels
// Frigate       2-4       1-2        VSmall    1.2-1.5m    1
// Destroyer     4-6       2-3        Small     1.5-1.8m    1-2
// Cruiser       6-10      3-5        Medium    1.8-2.2m    2
// Battlecruiser 10-14     5-7        Medium+   2.2-2.6m    2-3
// Battleship    14-20     7-10       Large     2.6-3.0m    3
// Capital       20-40     10-20      Large+    3.0-3.6m    3-6

static const InteriorBudget BUDGET_TABLE[] = {
    // Frigate (class 0)
    {  2,  4,   1,  2,   1.2f, 1.5f,  1,   9.0f,   1.5f, 1.5f, 2.5f },
    // Destroyer (class 1)
    {  4,  6,   2,  3,   1.5f, 1.8f,  2,  14.0f,   2.0f, 2.0f, 2.5f },
    // Cruiser (class 2)
    {  6, 10,   3,  5,   1.8f, 2.2f,  2,  20.0f,   2.5f, 2.5f, 2.8f },
    // Battlecruiser (class 3)
    { 10, 14,   5,  7,   2.2f, 2.6f,  3,  28.0f,   3.0f, 3.0f, 3.0f },
    // Battleship (class 4)
    { 14, 20,   7, 10,   2.6f, 3.0f,  3,  36.0f,   3.5f, 3.5f, 3.0f },
    // Capital (class 5)
    { 20, 40,  10, 20,   3.0f, 3.6f,  6,  48.0f,   4.0f, 4.0f, 3.5f },
};

static constexpr int BUDGET_TABLE_SIZE = 6;

// ── computeInteriorBudget ───────────────────────────────────────────

InteriorBudget computeInteriorBudget(int shipClass) {
    if (shipClass < 0) shipClass = 0;
    if (shipClass >= BUDGET_TABLE_SIZE) shipClass = BUDGET_TABLE_SIZE - 1;
    return BUDGET_TABLE[shipClass];
}

// ── getMandatoryRooms ───────────────────────────────────────────────

MandatoryRoomSet getMandatoryRooms(int shipClass) {
    MandatoryRoomSet m{};
    // Every ship has a cockpit and some form of engineering.
    m.cockpit     = true;
    m.engineering = true;

    if (shipClass >= 1) m.weaponControl = true;   // Destroyer+
    if (shipClass >= 2) m.crewQuarters  = true;   // Cruiser+
    if (shipClass >= 2) m.reactor       = true;   // Cruiser+
    if (shipClass >= 3) m.cargoBay      = true;   // Battlecruiser+
    if (shipClass >= 4) m.shieldCore    = true;   // Battleship+
    if (shipClass >= 5) m.hangar        = true;   // Capital+

    return m;
}

// ── Bridge selection matrix (newupload.txt §PCG Bridge Selection) ───

BridgeType selectBridgeType(int shipClass, DeterministicRNG& rng) {
    switch (shipClass) {
        case 0: // Frigate — always spine cockpit
            return BridgeType::SpineCockpit;
        case 1: // Destroyer
            return rng.chance(0.7f) ? BridgeType::SpineCockpit
                                     : BridgeType::Hammerhead;
        case 2: // Cruiser
            return rng.chance(0.5f) ? BridgeType::SpineCockpit
                                     : BridgeType::Hammerhead;
        case 3: // Battlecruiser
            return rng.chance(0.5f) ? BridgeType::Hammerhead
                                     : BridgeType::RaisedTower;
        case 4: // Battleship
        {
            float roll = rng.nextFloat();
            if (roll < 0.4f) return BridgeType::InlineArmored;
            if (roll < 0.7f) return BridgeType::Hammerhead;
            return BridgeType::RaisedTower;
        }
        case 5: // Capital
        {
            float roll = rng.nextFloat();
            if (roll < 0.35f) return BridgeType::InlineArmored;
            if (roll < 0.55f) return BridgeType::RaisedTower;
            if (roll < 0.75f) return BridgeType::DistributedCIC;
            return BridgeType::Hammerhead;
        }
        default:
            return BridgeType::SpineCockpit;
    }
}

// ── Name helpers ────────────────────────────────────────────────────

const char* bridgeTypeName(BridgeType type) {
    switch (type) {
        case BridgeType::InlineArmored:  return "Inline Armored";
        case BridgeType::Hammerhead:     return "Hammerhead";
        case BridgeType::RaisedTower:    return "Raised Tower";
        case BridgeType::SpineCockpit:   return "Spine Cockpit";
        case BridgeType::DistributedCIC: return "Distributed CIC";
        case BridgeType::Civilian:       return "Civilian";
    }
    return "Unknown";
}

const char* interiorRoomTypeName(InteriorRoomType type) {
    switch (type) {
        case InteriorRoomType::Cockpit:          return "Cockpit";
        case InteriorRoomType::Bridge:           return "Bridge";
        case InteriorRoomType::CICNode:          return "CIC Node";
        case InteriorRoomType::SensorCore:       return "Sensor Core";
        case InteriorRoomType::Engineering:      return "Engineering";
        case InteriorRoomType::Reactor:          return "Reactor";
        case InteriorRoomType::PowerRelay:       return "Power Relay";
        case InteriorRoomType::Coolant:          return "Coolant";
        case InteriorRoomType::LifeSupport:      return "Life Support";
        case InteriorRoomType::Comms:            return "Comms";
        case InteriorRoomType::WeaponControl:    return "Weapon Control";
        case InteriorRoomType::Magazine:         return "Magazine";
        case InteriorRoomType::ShieldCore:       return "Shield Core";
        case InteriorRoomType::CrewQuarters:     return "Crew Quarters";
        case InteriorRoomType::Barracks:         return "Barracks";
        case InteriorRoomType::Medbay:           return "Medbay";
        case InteriorRoomType::MessHall:         return "Mess Hall";
        case InteriorRoomType::CargoBay:         return "Cargo Bay";
        case InteriorRoomType::Hangar:           return "Hangar";
        case InteriorRoomType::DockingSpine:     return "Docking Spine";
        case InteriorRoomType::Corridor:         return "Corridor";
        case InteriorRoomType::MaintenanceShaft: return "Maintenance Shaft";
        case InteriorRoomType::EscapePod:        return "Escape Pod";
    }
    return "Unknown";
}

// ── Hull influence ──────────────────────────────────────────────────

HullInfluence computeHullInfluence(InteriorRoomType type,
                                    float posX, float posY, float posZ) {
    HullInfluence hi{};
    hi.roomType = type;

    switch (type) {
        case InteriorRoomType::Cockpit:
        case InteriorRoomType::Bridge:
            // Nose geometry — push forward.
            hi.dirX = 1.0f; hi.dirY = 0.0f; hi.dirZ = 0.0f;
            hi.magnitude = 1.2f;
            break;
        case InteriorRoomType::Engineering:
        case InteriorRoomType::Coolant:
            // Aft spine thickening.
            hi.dirX = -1.0f; hi.dirY = 0.0f; hi.dirZ = 0.0f;
            hi.magnitude = 1.5f;
            break;
        case InteriorRoomType::Reactor:
        case InteriorRoomType::PowerRelay:
            // Central mass expansion.
            hi.dirX = 0.0f; hi.dirY = 0.0f; hi.dirZ = 0.0f;
            hi.magnitude = 2.0f;
            break;
        case InteriorRoomType::WeaponControl:
        case InteriorRoomType::Magazine:
            // Lateral hardpoint bulge — direction depends on position.
            hi.dirX = 0.0f; hi.dirY = (posY >= 0.0f) ? 1.0f : -1.0f; hi.dirZ = 0.0f;
            hi.magnitude = 0.8f;
            break;
        case InteriorRoomType::Hangar:
        case InteriorRoomType::CargoBay:
            // Ventral bay.
            hi.dirX = 0.0f; hi.dirY = 0.0f; hi.dirZ = -1.0f;
            hi.magnitude = 1.8f;
            break;
        case InteriorRoomType::SensorCore:
        case InteriorRoomType::Comms:
            // Dorsal fin / antenna.
            hi.dirX = 0.0f; hi.dirY = 0.0f; hi.dirZ = 1.0f;
            hi.magnitude = 0.6f;
            break;
        case InteriorRoomType::ShieldCore:
            // Central expansion (shielding mass).
            hi.dirX = 0.0f; hi.dirY = 0.0f; hi.dirZ = 0.0f;
            hi.magnitude = 1.0f;
            break;
        default:
            // Habitation, corridors, etc. — negligible influence.
            hi.dirX = 0.0f; hi.dirY = 0.0f; hi.dirZ = 0.0f;
            hi.magnitude = 0.0f;
            break;
    }

    return hi;
}

} // namespace pcg
} // namespace atlas
