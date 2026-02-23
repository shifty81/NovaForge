#ifndef EVE_PCG_FLEET_DOCTRINE_H
#define EVE_PCG_FLEET_DOCTRINE_H

#include "pcg_context.h"
#include "ship_generator.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

/**
 * @brief Fleet doctrine types modelled after EVE fleet compositions.
 */
enum class FleetDoctrine : uint32_t {
    Brawler,         ///< Close-range, high-tank fleet.
    Sniper,          ///< Long-range DPS fleet.
    Kite,            ///< Fast, hit-and-run fleet.
    Logistics,       ///< Support-heavy fleet.
    CapitalSupport,  ///< Capital-backed fleet.
};

/**
 * @brief Role assigned to each ship in a fleet.
 */
enum class FleetRole : uint32_t {
    DPS,
    Tackle,
    Logistics,
    Scout,
    Commander,
};

/**
 * @brief A single slot in a generated fleet composition.
 */
struct FleetSlot {
    FleetRole   role;
    HullClass   hullClass;
    GeneratedShip ship;
};

/**
 * @brief Full fleet composition produced by the doctrine generator.
 */
struct GeneratedFleet {
    FleetDoctrine         doctrine;
    uint64_t              seed;
    std::vector<FleetSlot> slots;
    int                   totalShips;
};

/**
 * @brief Deterministic fleet composition generator.
 *
 * Given a doctrine and a ship count, produces a fleet where every ship
 * is procedurally generated with role-appropriate constraints.
 */
class FleetDoctrineGenerator {
public:
    /**
     * @brief Generate a fleet.
     * @param ctx       PCG context (seed + version).
     * @param doctrine  Desired fleet doctrine.
     * @param shipCount Number of ships to generate.
     */
    static GeneratedFleet generate(const PCGContext& ctx,
                                   FleetDoctrine doctrine,
                                   int shipCount);

private:
    struct RoleMix {
        float dps;
        float tackle;
        float logistics;
        float scout;
    };

    static RoleMix       getRoleMix(FleetDoctrine doctrine);
    static HullClass     hullForRole(FleetRole role, FleetDoctrine doctrine,
                                     DeterministicRNG& rng);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_FLEET_DOCTRINE_H
