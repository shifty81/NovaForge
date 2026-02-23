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
enum class HullClass : uint32_t {
    Frigate,
    Destroyer,
    Cruiser,
    Battlecruiser,
    Battleship,
    Capital,
};

// ── Weapon socket sizes ─────────────────────────────────────────────
enum class WeaponSize : uint32_t {
    Small,
    Medium,
    Large,
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
};

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
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_SHIP_GENERATOR_H
