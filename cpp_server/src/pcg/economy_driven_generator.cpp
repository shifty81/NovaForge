#include "pcg/economy_driven_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Role mix per economy state ──────────────────────────────────────

EconomyDrivenGenerator::RoleMix EconomyDrivenGenerator::getRoleMix(
        EconomyState economy) {
    // Each float is the relative weight for that role.
    switch (economy) {
        case EconomyState::ResourceRich:
            return { 0.35f, 0.25f, 0.10f, 0.05f, 0.05f, 0.10f, 0.10f };
        case EconomyState::WarTorn:
            return { 0.00f, 0.05f, 0.30f, 0.15f, 0.10f, 0.05f, 0.35f };
        case EconomyState::Prosperous:
            return { 0.05f, 0.20f, 0.20f, 0.05f, 0.05f, 0.30f, 0.15f };
        case EconomyState::Declining:
            return { 0.10f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f };
        case EconomyState::Lawless:
            return { 0.00f, 0.05f, 0.00f, 0.45f, 0.25f, 0.05f, 0.20f };
    }
    return { 0.10f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f, 0.15f };
}

// ── Role selection via cumulative weights ───────────────────────────

EconomyShipRole EconomyDrivenGenerator::selectRole(DeterministicRNG& rng,
                                                    const RoleMix& mix) {
    float roll = rng.nextFloat();
    float cum  = 0.0f;
    cum += mix.miner;    if (roll < cum) return EconomyShipRole::Miner;
    cum += mix.hauler;   if (roll < cum) return EconomyShipRole::Hauler;
    cum += mix.patrol;   if (roll < cum) return EconomyShipRole::Patrol;
    cum += mix.pirate;   if (roll < cum) return EconomyShipRole::Pirate;
    cum += mix.scavenger;if (roll < cum) return EconomyShipRole::Scavenger;
    cum += mix.trader;   if (roll < cum) return EconomyShipRole::Trader;
    return EconomyShipRole::MilitaryEscort;
}

// ── Hull class for role ─────────────────────────────────────────────

HullClass EconomyDrivenGenerator::hullForRole(DeterministicRNG& rng,
                                               EconomyShipRole role) {
    float r = rng.nextFloat();
    switch (role) {
        case EconomyShipRole::Miner:
            // Mining vessels: mostly cruiser / battlecruiser sized.
            if (r < 0.3f)  return HullClass::Cruiser;
            if (r < 0.7f)  return HullClass::Battlecruiser;
            return HullClass::Battleship;
        case EconomyShipRole::Hauler:
            if (r < 0.3f)  return HullClass::Destroyer;
            if (r < 0.7f)  return HullClass::Cruiser;
            return HullClass::Battleship;
        case EconomyShipRole::Patrol:
            if (r < 0.4f)  return HullClass::Frigate;
            if (r < 0.7f)  return HullClass::Destroyer;
            return HullClass::Cruiser;
        case EconomyShipRole::Pirate:
            if (r < 0.5f)  return HullClass::Frigate;
            if (r < 0.8f)  return HullClass::Destroyer;
            return HullClass::Cruiser;
        case EconomyShipRole::Scavenger:
            if (r < 0.5f)  return HullClass::Frigate;
            if (r < 0.8f)  return HullClass::Destroyer;
            return HullClass::Cruiser;
        case EconomyShipRole::Trader:
            if (r < 0.2f)  return HullClass::Destroyer;
            if (r < 0.6f)  return HullClass::Cruiser;
            return HullClass::Battlecruiser;
        case EconomyShipRole::MilitaryEscort:
            if (r < 0.2f)  return HullClass::Destroyer;
            if (r < 0.5f)  return HullClass::Cruiser;
            if (r < 0.8f)  return HullClass::Battlecruiser;
            return HullClass::Battleship;
    }
    return HullClass::Cruiser;
}

// ── Equipment quality ───────────────────────────────────────────────

float EconomyDrivenGenerator::equipmentQuality(DeterministicRNG& rng,
                                                EconomyState economy,
                                                EconomyShipRole role) {
    float base = 0.5f;
    switch (economy) {
        case EconomyState::Prosperous:   base = 0.7f; break;
        case EconomyState::ResourceRich: base = 0.6f; break;
        case EconomyState::WarTorn:      base = 0.5f; break;
        case EconomyState::Declining:    base = 0.3f; break;
        case EconomyState::Lawless:      base = 0.4f; break;
    }

    // Military and patrol ships tend to have better equipment.
    if (role == EconomyShipRole::MilitaryEscort || role == EconomyShipRole::Patrol) {
        base += 0.1f;
    }
    // Scavengers and pirates have mixed quality.
    if (role == EconomyShipRole::Scavenger) {
        base -= 0.1f;
    }

    float variation = rng.rangeFloat(-0.1f, 0.1f);
    return std::max(0.0f, std::min(1.0f, base + variation));
}

// ── Damage wear (pre-existing damage) ──────────────────────────────

float EconomyDrivenGenerator::damageWear(DeterministicRNG& rng,
                                          EconomyState economy) {
    switch (economy) {
        case EconomyState::Prosperous:
            return rng.rangeFloat(0.0f, 0.05f);   // Minimal wear.
        case EconomyState::ResourceRich:
            return rng.rangeFloat(0.0f, 0.10f);   // Working equipment.
        case EconomyState::WarTorn:
            return rng.rangeFloat(0.15f, 0.60f);  // Battle-scarred.
        case EconomyState::Declining:
            return rng.rangeFloat(0.05f, 0.30f);  // Aging fleet.
        case EconomyState::Lawless:
            return rng.rangeFloat(0.10f, 0.45f);  // Rough condition.
    }
    return 0.0f;
}

// ── Public API ──────────────────────────────────────────────────────

std::string EconomyDrivenGenerator::economyStateName(EconomyState state) {
    switch (state) {
        case EconomyState::Prosperous:   return "Prosperous";
        case EconomyState::ResourceRich: return "ResourceRich";
        case EconomyState::WarTorn:      return "WarTorn";
        case EconomyState::Declining:    return "Declining";
        case EconomyState::Lawless:      return "Lawless";
    }
    return "Unknown";
}

std::string EconomyDrivenGenerator::shipRoleName(EconomyShipRole role) {
    switch (role) {
        case EconomyShipRole::Miner:          return "Miner";
        case EconomyShipRole::Hauler:         return "Hauler";
        case EconomyShipRole::Patrol:         return "Patrol";
        case EconomyShipRole::Pirate:         return "Pirate";
        case EconomyShipRole::Scavenger:      return "Scavenger";
        case EconomyShipRole::Trader:         return "Trader";
        case EconomyShipRole::MilitaryEscort: return "MilitaryEscort";
    }
    return "Unknown";
}

GeneratedEconomyFleet EconomyDrivenGenerator::generate(
        const PCGContext& ctx,
        EconomyState economy,
        int shipCount) {
    DeterministicRNG rng(ctx.seed);

    shipCount = std::max(1, std::min(shipCount, 50));

    GeneratedEconomyFleet fleet{};
    fleet.fleet_id    = ctx.seed;
    fleet.economy     = economy;
    fleet.total_ships = shipCount;

    RoleMix mix = getRoleMix(economy);

    float totalQuality = 0.0f;

    fleet.ships.reserve(static_cast<size_t>(shipCount));
    for (int i = 0; i < shipCount; ++i) {
        EconomyShip es{};

        // 1. Select role based on economy.
        es.role = selectRole(rng, mix);

        // 2. Choose hull class for this role.
        HullClass hull = hullForRole(rng, es.role);

        // 3. Generate the base ship deterministically.
        uint64_t shipSeed = deriveSeed(ctx.seed, static_cast<uint64_t>(i));
        PCGContext shipCtx{ shipSeed, ctx.version };
        es.base = ShipGenerator::generate(shipCtx, hull);

        // 4. Equipment quality.
        es.equipment_quality = equipmentQuality(rng, economy, es.role);
        totalQuality += es.equipment_quality;

        // 5. Pre-existing damage / wear.
        es.damage_wear = damageWear(rng, economy);

        // 6. Armed status (miners / haulers may be unarmed).
        if (es.role == EconomyShipRole::Miner || es.role == EconomyShipRole::Hauler) {
            es.is_armed = rng.chance(0.3f);
        } else {
            es.is_armed = true;
        }

        fleet.ships.push_back(es);
    }

    fleet.average_equipment_quality = (shipCount > 0)
        ? totalQuality / static_cast<float>(shipCount)
        : 0.0f;

    fleet.valid = !fleet.ships.empty();

    return fleet;
}

} // namespace pcg
} // namespace atlas
