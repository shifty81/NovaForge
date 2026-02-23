#ifndef EVE_PCG_INTERIOR_BUDGET_H
#define EVE_PCG_INTERIOR_BUDGET_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

// ── Bridge / cockpit class types ────────────────────────────────────
enum class BridgeType : uint32_t {
    InlineArmored,    ///< Buried command bridge (titan/dread).
    Hammerhead,       ///< Forward-mounted, exposed, high-vis.
    RaisedTower,      ///< Elevated superstructure (carrier).
    SpineCockpit,     ///< Inline spine cockpit (frigate/escort).
    DistributedCIC,   ///< No single bridge — multiple hidden nodes.
    Civilian,         ///< Large windows, minimal armor (hauler).
};

// ── Extended room types (full taxonomy from newupload.txt) ──────────
enum class InteriorRoomType : uint32_t {
    // Command & Control
    Cockpit,
    Bridge,
    CICNode,           ///< Distributed command node.
    SensorCore,
    // Engineering & Power
    Engineering,
    Reactor,
    PowerRelay,
    Coolant,
    // Utilities & Support
    LifeSupport,
    Comms,
    // Combat Systems
    WeaponControl,
    Magazine,
    ShieldCore,
    // Habitation
    CrewQuarters,
    Barracks,
    Medbay,
    MessHall,
    // Logistics
    CargoBay,
    Hangar,
    DockingSpine,
    // Circulation
    Corridor,
    MaintenanceShaft,
    EscapePod,
};

/**
 * @brief Interior budget derived from ship class + hull volume.
 *
 * Controls how many rooms, corridors, and vertical levels a ship
 * may contain.  Every ship from Frigate to Capital gets an interior;
 * size and class only determine how large.
 */
struct InteriorBudget {
    int   roomCountMin;
    int   roomCountMax;
    int   corridorCountMin;
    int   corridorCountMax;
    float corridorWidthMin;   ///< metres
    float corridorWidthMax;   ///< metres
    int   maxVerticalLevels;
    float avgRoomArea;        ///< m²
    float minCockpitWidth;    ///< metres — hard minimum
    float minCockpitDepth;    ///< metres — hard minimum
    float minCockpitHeight;   ///< metres — hard minimum
};

/**
 * @brief Hull influence emitted by a room onto the exterior mesh.
 *
 * Each room pushes the hull outwards in a direction relative to the
 * ship spine.  WeaponControl → hardpoint bulge, Hangar → ventral bay,
 * Engineering → rear spine thickening, etc.
 */
struct HullInfluence {
    InteriorRoomType roomType;
    float dirX, dirY, dirZ;   ///< Outward direction from spine.
    float magnitude;           ///< How much hull expands (metres).
};

/**
 * @brief Mandatory rooms that must appear for a given ship class.
 */
struct MandatoryRoomSet {
    bool cockpit;
    bool engineering;
    bool reactor;
    bool weaponControl;
    bool crewQuarters;
    bool cargoBay;
    bool hangar;
    bool shieldCore;
};

// ── Ship-class to bridge-type selection matrix ──────────────────────

/**
 * @brief Allowed bridge types per ship class (0=Frigate .. 5=Capital).
 */
struct BridgeAllowance {
    int               shipClass;
    std::vector<BridgeType> allowed;
};

// ── API ─────────────────────────────────────────────────────────────

/**
 * @brief Compute the interior budget for a given ship class.
 *
 * Maps the scaling table from newupload.txt:
 *   Frigate:       2-4 rooms,  1-2 corridors,  1.2-1.5m wide, 1 level
 *   Destroyer:     4-6 rooms,  2-3 corridors,  1.5-1.8m wide, 1-2 levels
 *   Cruiser:       6-10 rooms, 3-5 corridors,  1.8-2.2m wide, 2 levels
 *   Battlecruiser: 10-14 rooms,5-7 corridors,  2.2-2.6m wide, 2-3 levels
 *   Battleship:    14-20 rooms,7-10 corridors,  2.6-3.0m wide, 3 levels
 *   Capital:       20-40 rooms,10-20 corridors, 3.0m+ wide,    3-6 levels
 *
 * @param shipClass  0=Frigate .. 5=Capital.
 */
InteriorBudget computeInteriorBudget(int shipClass);

/**
 * @brief Get the mandatory rooms for a ship class.
 */
MandatoryRoomSet getMandatoryRooms(int shipClass);

/**
 * @brief Select a bridge type for the ship class (deterministic).
 */
BridgeType selectBridgeType(int shipClass, DeterministicRNG& rng);

/**
 * @brief Human-readable name for a bridge type.
 */
const char* bridgeTypeName(BridgeType type);

/**
 * @brief Human-readable name for an interior room type.
 */
const char* interiorRoomTypeName(InteriorRoomType type);

/**
 * @brief Compute hull influence for a room based on its type.
 *
 * Rooms push the hull outwards in type-specific directions:
 *   WeaponControl → lateral hardpoint bulge
 *   Hangar        → ventral bay
 *   Engineering   → aft spine thickening
 *   Cockpit       → nose geometry
 *   Reactor       → central mass expansion
 *   SensorCore    → dorsal fin / antenna
 */
HullInfluence computeHullInfluence(InteriorRoomType type,
                                    float posX, float posY, float posZ);

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_INTERIOR_BUDGET_H
