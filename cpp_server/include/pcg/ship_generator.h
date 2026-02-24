#ifndef EVE_PCG_SHIP_GENERATOR_H
#define EVE_PCG_SHIP_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

class PCGTraceRecorder;  // forward declaration

// ── Hull classes ────────────────────────────────────────────────────
// Values 0–5 match the original enum for backward compatibility.
// New Tech II, Industrial, and Capital-specific classes start at 6.
enum class HullClass : uint32_t {
    // ── Tech I base hulls ───────────────────
    Frigate         = 0,
    Destroyer       = 1,
    Cruiser         = 2,
    Battlecruiser   = 3,
    Battleship      = 4,
    Capital         = 5,  // Generic capital (kept for compat)
    // ── Tech II frigate variants ────────────
    Interceptor     = 6,
    CovertOps       = 7,
    AssaultFrigate  = 8,
    StealthBomber   = 9,
    // ── Tech II cruiser variants ────────────
    Logistics       = 10,
    Recon           = 11,
    // ── Tech II battlecruiser variants ──────
    CommandShip     = 12,
    // ── Tech II battleship variants ─────────
    Marauder        = 13,
    // ── Industrial ──────────────────────────
    Industrial      = 14,
    MiningBarge     = 15,
    Exhumer         = 16,
    // ── Capital (specific) ──────────────────
    Carrier         = 17,
    Dreadnought     = 18,
    Titan           = 19,
};

/// Total number of hull classes (used for template array sizing).
static constexpr int HULL_CLASS_COUNT = 20;

// ── Weapon socket sizes ─────────────────────────────────────────────
enum class WeaponSize : uint32_t {
    Small,
    Medium,
    Large,
    XLarge,   // Capital-class weapons (dreadnoughts, titans).
};

// ── Generated ship data ─────────────────────────────────────────────
struct GeneratedShip {
    uint64_t    shipId;
    HullClass   hullClass;
    float       mass;          // tonnes
    float       thrust;        // newtons
    float       alignTime;     // seconds (derived: mass / thrust)
    float       capacitor;     // GJ
    float       powergrid;     // MW (total available)
    float       cpu;           // tf (total available)
    int         turretSlots;
    int         launcherSlots;
    int         engineCount;
    WeaponSize  maxWeaponSize;
    bool        valid;         // true if all constraints passed
    float       armorHP;       // hit points
    float       shieldHP;      // hit points
    float       signatureRadius; // metres
    float       targetingSpeed;  // seconds to lock
    int         droneBay;      // m³ capacity (0 for no drones)
    int         cargoCapacity; // m³ cargo hold
    int         techLevel;     // 1 = Tech I, 2 = Tech II
    std::string shipName;      // procedurally generated name
};

/**
 * @brief Return the Tech I base hull class for any hull.
 *
 * Tech II variants map to their parent T1 class so that downstream
 * systems (spine hull, damage state, turret placement) can reuse
 * the same geometry / scaling logic.
 */
inline HullClass baseHullClass(HullClass hull) {
    switch (hull) {
        case HullClass::Interceptor:
        case HullClass::CovertOps:
        case HullClass::AssaultFrigate:
        case HullClass::StealthBomber:
            return HullClass::Frigate;
        case HullClass::Logistics:
        case HullClass::Recon:
            return HullClass::Cruiser;
        case HullClass::CommandShip:
            return HullClass::Battlecruiser;
        case HullClass::Marauder:
            return HullClass::Battleship;
        case HullClass::Industrial:
        case HullClass::MiningBarge:
        case HullClass::Exhumer:
            return HullClass::Cruiser;
        case HullClass::Carrier:
        case HullClass::Dreadnought:
        case HullClass::Titan:
            return HullClass::Capital;
        default:
            return hull;  // Already a base class.
    }
}

/** @brief Return tech level (1 or 2) for the given hull class. */
inline int hullTechLevel(HullClass hull) {
    switch (hull) {
        case HullClass::Interceptor:
        case HullClass::CovertOps:
        case HullClass::AssaultFrigate:
        case HullClass::StealthBomber:
        case HullClass::Logistics:
        case HullClass::Recon:
        case HullClass::CommandShip:
        case HullClass::Marauder:
        case HullClass::Exhumer:
            return 2;
        default:
            return 1;
    }
}

/**
 * @brief Deterministic ship generator.
 *
 * Given a PCGContext the generator produces a fully-specified
 * GeneratedShip.  The same context always yields the same ship.
 *
 * Generation follows the hierarchy:
 *   1. Select hull class
 *   2. Derive mass & fitting budgets
 *   3. Attach engines (thrust ≥ mass × min-accel)
 *   4. Attach weapons (within powergrid & slot limits)
 *   5. Validate constraints
 */
class ShipGenerator {
public:
    /** Generate a ship from the given context. */
    static GeneratedShip generate(const PCGContext& ctx);

    /** Generate with an explicit hull class override. */
    static GeneratedShip generate(const PCGContext& ctx, HullClass hull);

    /** Set a trace recorder for debug visualization (nullable). */
    static void setTraceRecorder(PCGTraceRecorder* recorder);

    /** Human-readable hull class name. */
    static std::string hullClassName(HullClass hull);

private:
    static PCGTraceRecorder* traceRecorder_;

    static HullClass   selectHull(DeterministicRNG& rng);
    static void        deriveStats(DeterministicRNG& rng, GeneratedShip& ship);
    static void        attachEngines(DeterministicRNG& rng, GeneratedShip& ship);
    static void        attachWeapons(DeterministicRNG& rng, GeneratedShip& ship);
    static bool        validateConstraints(const GeneratedShip& ship);
    static std::string generateName(DeterministicRNG& rng, HullClass hull);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SHIP_GENERATOR_H
