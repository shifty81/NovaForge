#ifndef EVE_PCG_VERSION_H
#define EVE_PCG_VERSION_H

#include <cstdint>

namespace atlas {
namespace pcg {

/**
 * @brief Per-domain rule versions for save-safe PCG migration.
 *
 * When generation rules change (e.g. new weapon balancing), the
 * version for that domain is incremented.  Old saves regenerate
 * objects using the old version's logic, while new objects use the
 * latest version.
 *
 * This struct is stored in every save file header so that the engine
 * knows exactly which rule set to apply.
 *
 * Non-negotiable rule: changing PCG logic without bumping the version
 * will silently break existing save files.
 */
struct PCGVersion {
    uint32_t ship_rules_version      = 1;
    uint32_t asteroid_rules_version  = 1;
    uint32_t station_rules_version   = 1;
    uint32_t fleet_rules_version     = 1;
    uint32_t npc_rules_version       = 1;
    uint32_t anomaly_rules_version   = 1;
    uint32_t loot_rules_version      = 1;
    uint32_t capital_rules_version   = 1;
    uint32_t weapon_rules_version    = 1;

    /** Quick equality check for save-file validation. */
    bool operator==(const PCGVersion& other) const {
        return ship_rules_version     == other.ship_rules_version
            && asteroid_rules_version == other.asteroid_rules_version
            && station_rules_version  == other.station_rules_version
            && fleet_rules_version    == other.fleet_rules_version
            && npc_rules_version      == other.npc_rules_version
            && anomaly_rules_version  == other.anomaly_rules_version
            && loot_rules_version     == other.loot_rules_version
            && capital_rules_version  == other.capital_rules_version
            && weapon_rules_version   == other.weapon_rules_version;
    }

    bool operator!=(const PCGVersion& other) const {
        return !(*this == other);
    }
};

/**
 * @brief Current rule versions (bump when generation logic changes).
 *
 * Usage:
 *   PCGContext ctx = mgr.makeContext(
 *       PCGDomain::Ship,
 *       sectorSeed,
 *       shipId,
 *       CURRENT_PCG_VERSION.ship_rules_version
 *   );
 */
inline constexpr PCGVersion CURRENT_PCG_VERSION = {
    /* ship      */ 1,
    /* asteroid  */ 1,
    /* station   */ 1,
    /* fleet     */ 1,
    /* npc       */ 1,
    /* anomaly   */ 1,
    /* loot      */ 1,
    /* capital   */ 1,
    /* weapon    */ 1,
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_VERSION_H
