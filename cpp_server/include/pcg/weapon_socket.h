#ifndef EVE_PCG_WEAPON_SOCKET_H
#define EVE_PCG_WEAPON_SOCKET_H

#include "ship_generator.h"
#include <cstdint>
#include <cmath>
#include <vector>

namespace atlas {
namespace pcg {

/**
 * @brief Describes a single weapon mount point on a ship module.
 *
 * Sockets are placed during the modular-assembly pass and carry both
 * physical constraints (size, arc) and functional metadata used by the
 * constraint solver and AI firing-arc logic.
 */
struct WeaponSocket {
    uint32_t    id;
    WeaponSize  size;           ///< Small / Medium / Large.
    float       directionDeg;   ///< Nominal facing (degrees, 0 = forward).
    float       arcDeg;         ///< Total arc width (degrees).
    float       tracking;       ///< Tracking speed (rad/s).
    bool        occupied;       ///< true once a weapon is fitted.
};

/**
 * @brief Result of a weapon-placement pass for a ship.
 */
struct WeaponLayout {
    uint64_t                  shipId;
    std::vector<WeaponSocket> sockets;
    float                     totalDPS;
    float                     forwardDPS;
    float                     turretOverlap; ///< Fraction [0,1] of arc overlap.
};

/**
 * @brief Utility functions for weapon-socket math.
 *
 * All angles are in degrees unless noted otherwise.  Internally the
 * helpers convert to radians where needed.
 */
class WeaponSocketUtil {
public:
    /**
     * @brief Can a turret track the target at the given angular velocity?
     *
     * Direct EVE analog:  hit iff angularVelocity ≤ tracking.
     *
     * @param angularVelocity  Target's angular velocity (rad/s).
     * @param tracking         Turret tracking speed (rad/s).
     * @return true if the turret can track fast enough.
     */
    static bool canHitTarget(float angularVelocity, float tracking);

    /**
     * @brief Is a bearing inside a socket's firing arc?
     *
     * @param bearingDeg  Bearing to target (degrees, 0 = forward).
     * @param socket      The weapon socket.
     * @return true if the target falls within the arc.
     */
    static bool isInArc(float bearingDeg, const WeaponSocket& socket);

    /**
     * @brief Calculate overlap fraction between two socket arcs.
     * @return Fraction in [0,1] — 0 means no overlap, 1 means identical.
     */
    static float arcOverlap(const WeaponSocket& a, const WeaponSocket& b);

    /**
     * @brief Generate a weapon layout for the given ship.
     *
     * Places sockets according to the ship's hull class and role:
     *   - Brawlers → forward batteries (narrow arc).
     *   - Capitals → limited traverse (wide spacing).
     *   - Escorts  → wide arcs.
     *
     * @param ctx   PCG context for deterministic placement.
     * @param ship  The generated ship to place weapons on.
     * @return A fully populated WeaponLayout.
     */
    static WeaponLayout generateLayout(const PCGContext& ctx,
                                       const GeneratedShip& ship);

private:
    /** Normalise an angle into [0, 360). */
    static float normalizeDeg(float deg);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_WEAPON_SOCKET_H
