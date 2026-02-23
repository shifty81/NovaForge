#ifndef EVE_PCG_ECONOMY_DRIVEN_GENERATOR_H
#define EVE_PCG_ECONOMY_DRIVEN_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include "ship_generator.h"
#include <cstdint>
#include <string>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Economic state of a star system that drives NPC generation.
 */
enum class EconomyState : uint32_t {
    Prosperous,    ///< Wealthy trade hub — sleek haulers, well-armed patrols.
    ResourceRich,  ///< Mining bonanza — bulky mining ships, ore haulers.
    WarTorn,       ///< Active conflict — damaged patrols, military convoys.
    Declining,     ///< Fading economy — older ships, fewer NPCs.
    Lawless,       ///< No law — pirate gangs, scavengers, no patrols.
};

/**
 * @brief NPC ship role adapted to the system economy.
 */
enum class EconomyShipRole : uint32_t {
    Miner,         ///< Mining vessel (barges, haulers).
    Hauler,        ///< Cargo transport.
    Patrol,        ///< Security patrol.
    Pirate,        ///< Hostile aggressor.
    Scavenger,     ///< Wreck salvager.
    Trader,        ///< Merchant vessel.
    MilitaryEscort,///< Armed military ship.
};

/**
 * @brief A single NPC ship generated with economy context.
 */
struct EconomyShip {
    GeneratedShip    base;
    EconomyShipRole  role;
    float            equipment_quality;  ///< 0.0 (junk) – 1.0 (top-tier).
    float            damage_wear;        ///< Pre-existing damage [0,1] (war-torn ships).
    bool             is_armed;
};

/**
 * @brief Fleet of NPC ships generated to match system economy.
 */
struct GeneratedEconomyFleet {
    uint64_t                    fleet_id;
    EconomyState                economy;
    std::vector<EconomyShip>    ships;
    int                         total_ships;
    float                       average_equipment_quality;
    bool                        valid;
};

/**
 * @brief Economy-driven NPC ship generator (Phase 12).
 *
 * Produces NPC fleets whose composition reflects the star system's
 * economic state:
 *   - ResourceRich → mining barges and ore haulers
 *   - WarTorn      → damaged military patrols and convoys
 *   - Prosperous   → sleek traders and well-equipped patrols
 *   - Declining    → older, under-equipped ships
 *   - Lawless      → pirate gangs and scavengers
 *
 * Deterministic: same seed + economy state → same fleet.
 */
class EconomyDrivenGenerator {
public:
    /**
     * @brief Generate an economy-adapted NPC fleet.
     * @param ctx        PCG context (seed + version).
     * @param economy    System economy state.
     * @param shipCount  Number of ships to generate.
     */
    static GeneratedEconomyFleet generate(const PCGContext& ctx,
                                          EconomyState economy,
                                          int shipCount);

    /** Human-readable economy state name. */
    static std::string economyStateName(EconomyState state);

    /** Human-readable economy ship role name. */
    static std::string shipRoleName(EconomyShipRole role);

private:
    struct RoleMix {
        float miner;
        float hauler;
        float patrol;
        float pirate;
        float scavenger;
        float trader;
        float military;
    };

    static RoleMix         getRoleMix(EconomyState economy);
    static EconomyShipRole selectRole(DeterministicRNG& rng,
                                      const RoleMix& mix);
    static HullClass       hullForRole(DeterministicRNG& rng,
                                       EconomyShipRole role);
    static float           equipmentQuality(DeterministicRNG& rng,
                                            EconomyState economy,
                                            EconomyShipRole role);
    static float           damageWear(DeterministicRNG& rng,
                                      EconomyState economy);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_ECONOMY_DRIVEN_GENERATOR_H
