#include "pcg/fleet_doctrine.h"
#include "pcg/hash_utils.h"

namespace atlas {
namespace pcg {

// ── Role-mix tables per doctrine ───────────────────────────────────

FleetDoctrineGenerator::RoleMix
FleetDoctrineGenerator::getRoleMix(FleetDoctrine doctrine) {
    switch (doctrine) {
        case FleetDoctrine::Brawler:
            return { 0.60f, 0.20f, 0.15f, 0.05f };
        case FleetDoctrine::Sniper:
            return { 0.70f, 0.05f, 0.15f, 0.10f };
        case FleetDoctrine::Kite:
            return { 0.50f, 0.25f, 0.10f, 0.15f };
        case FleetDoctrine::Logistics:
            return { 0.30f, 0.10f, 0.50f, 0.10f };
        case FleetDoctrine::CapitalSupport:
            return { 0.40f, 0.15f, 0.20f, 0.25f };
    }
    return { 0.60f, 0.20f, 0.15f, 0.05f }; // fallback
}

HullClass FleetDoctrineGenerator::hullForRole(FleetRole role,
                                               FleetDoctrine doctrine,
                                               DeterministicRNG& rng) {
    switch (role) {
        case FleetRole::Tackle:
            // Interceptors are the preferred tackle ship (T2).
            if (rng.chance(0.4f)) return HullClass::Interceptor;
            return rng.chance(0.5f) ? HullClass::Frigate : HullClass::Destroyer;

        case FleetRole::Scout:
            // Covert ops for scouting.
            if (rng.chance(0.5f)) return HullClass::CovertOps;
            return rng.chance(0.6f) ? HullClass::Frigate : HullClass::Interceptor;

        case FleetRole::Logistics:
            // T2 logistics cruisers are preferred.
            if (rng.chance(0.6f)) return HullClass::Logistics;
            return rng.chance(0.7f) ? HullClass::Cruiser : HullClass::Battlecruiser;

        case FleetRole::DPS:
            if (doctrine == FleetDoctrine::CapitalSupport) {
                float r = rng.nextFloat();
                if (r < 0.15f) return HullClass::Dreadnought;
                if (r < 0.25f) return HullClass::Carrier;
                if (r < 0.50f) return HullClass::Battleship;
                return HullClass::Battlecruiser;
            }
            if (doctrine == FleetDoctrine::Brawler) {
                if (rng.chance(0.2f)) return HullClass::Marauder;
                return rng.chance(0.5f) ? HullClass::Battleship : HullClass::Battlecruiser;
            }
            if (doctrine == FleetDoctrine::Sniper) {
                if (rng.chance(0.15f)) return HullClass::Marauder;
                return rng.chance(0.6f) ? HullClass::Battleship : HullClass::Cruiser;
            }
            if (doctrine == FleetDoctrine::Kite) {
                float r = rng.nextFloat();
                if (r < 0.25f) return HullClass::AssaultFrigate;
                if (r < 0.50f) return HullClass::Cruiser;
                return HullClass::Battlecruiser;
            }
            return rng.chance(0.5f) ? HullClass::Cruiser : HullClass::Battlecruiser;

        case FleetRole::Commander:
            // Command ships are the ideal FC platform.
            if (rng.chance(0.6f)) return HullClass::CommandShip;
            return HullClass::Battlecruiser;
    }
    return HullClass::Cruiser;
}

// ── Public API ─────────────────────────────────────────────────────

GeneratedFleet FleetDoctrineGenerator::generate(const PCGContext& ctx,
                                                 FleetDoctrine doctrine,
                                                 int shipCount) {
    GeneratedFleet fleet;
    fleet.doctrine   = doctrine;
    fleet.seed       = ctx.seed;
    fleet.totalShips = shipCount;

    if (shipCount <= 0) return fleet;

    DeterministicRNG rng(ctx.seed);
    RoleMix mix = getRoleMix(doctrine);

    // Compute counts per role.
    int dpsCount  = static_cast<int>(static_cast<float>(shipCount) * mix.dps);
    int tackCount = static_cast<int>(static_cast<float>(shipCount) * mix.tackle);
    int logiCount = static_cast<int>(static_cast<float>(shipCount) * mix.logistics);
    int scoutCount = shipCount - dpsCount - tackCount - logiCount;
    if (scoutCount < 0) scoutCount = 0;

    // Always have exactly one commander (takes from DPS pool).
    if (dpsCount > 0) dpsCount--;

    // Helper lambda to add ships.
    auto addShips = [&](FleetRole role, int count) {
        for (int i = 0; i < count; ++i) {
            FleetSlot slot;
            slot.role      = role;
            slot.hullClass = hullForRole(role, doctrine, rng);

            // Derive per-ship context.
            uint64_t shipSeed = deriveSeed(ctx.seed,
                static_cast<uint64_t>(fleet.slots.size()));
            PCGContext shipCtx{ shipSeed, ctx.version };
            slot.ship = ShipGenerator::generate(shipCtx, slot.hullClass);

            fleet.slots.push_back(slot);
        }
    };

    // Commander first.
    {
        FleetSlot cmd;
        cmd.role      = FleetRole::Commander;
        cmd.hullClass = hullForRole(FleetRole::Commander, doctrine, rng);
        uint64_t cmdSeed = deriveSeed(ctx.seed, 0xFFFFFFFFULL);
        PCGContext cmdCtx{ cmdSeed, ctx.version };
        cmd.ship = ShipGenerator::generate(cmdCtx, cmd.hullClass);
        fleet.slots.push_back(cmd);
    }

    addShips(FleetRole::DPS, dpsCount);
    addShips(FleetRole::Tackle, tackCount);
    addShips(FleetRole::Logistics, logiCount);
    addShips(FleetRole::Scout, scoutCount);

    return fleet;
}

} // namespace pcg
} // namespace atlas
