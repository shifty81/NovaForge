#ifndef EVE_PCG_VERIFY_H
#define EVE_PCG_VERIFY_H

#include "hash_utils.h"
#include "ship_generator.h"
#include <cstdint>
#include <cstring>

namespace atlas {
namespace pcg {

/**
 * @brief Server-authoritative PCG verification for multiplayer.
 *
 * After a client regenerates an object locally it computes a
 * verification hash and sends  { objectId, hash }  to the server.
 * The server regenerates the same object and compares hashes.
 *
 *   Match    → OK
 *   Mismatch → resync or kick
 *
 * The hash is a deterministic function of the object's gameplay-
 * relevant fields (mass, thrust, weapon layout …).  Cosmetic data
 * (greeble) is excluded so that visual-only changes don't trigger
 * false mismatches.
 */
class PCGVerify {
public:
    /**
     * @brief Compute a verification hash for a generated ship.
     *
     * Only gameplay-critical fields are included:
     *   mass, thrust, capacitor, powergrid, cpu, turrets, launchers.
     */
    static uint64_t hashShip(const GeneratedShip& ship);

    /**
     * @brief Compare a client-reported hash against the server's
     *        locally generated hash.
     * @return true if hashes match (client is in sync).
     */
    static bool verifyShip(uint64_t clientHash,
                           const GeneratedShip& serverShip);

private:
    /** Reinterpret a float as its uint32 bit pattern (portable). */
    static uint64_t floatBits(float f);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_VERIFY_H
