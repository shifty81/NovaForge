#include "pcg/pcg_verify.h"

namespace atlas {
namespace pcg {

uint64_t PCGVerify::floatBits(float f) {
    uint32_t bits;
    std::memcpy(&bits, &f, sizeof(bits));
    return static_cast<uint64_t>(bits);
}

uint64_t PCGVerify::hashShip(const GeneratedShip& ship) {
    // Hash only gameplay-critical fields — cosmetic data is excluded
    // so that greeble or visual-only changes never cause desyncs.
    uint64_t h = hash64(
        floatBits(ship.mass),
        floatBits(ship.thrust),
        floatBits(ship.capacitor),
        floatBits(ship.powergrid)
    );

    h = hashCombine(h, floatBits(ship.cpu));
    h = hashCombine(h, static_cast<uint64_t>(ship.turretSlots));
    h = hashCombine(h, static_cast<uint64_t>(ship.launcherSlots));
    h = hashCombine(h, static_cast<uint64_t>(ship.engineCount));
    h = hashCombine(h, static_cast<uint64_t>(ship.maxWeaponSize));

    return h;
}

bool PCGVerify::verifyShip(uint64_t clientHash,
                            const GeneratedShip& serverShip) {
    return clientHash == hashShip(serverShip);
}

} // namespace pcg
} // namespace atlas
