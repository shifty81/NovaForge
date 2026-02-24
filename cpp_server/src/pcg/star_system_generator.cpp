#include "pcg/star_system_generator.h"
#include "pcg/hash_utils.h"

#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Star-class base templates ──────────────────────────────────────
struct StarTemplate {
    float lumMin, lumMax;       // solar luminosities
    float tempMin, tempMax;     // kelvin
    float radiusMin, radiusMax; // solar radii
    int   orbitMin, orbitMax;   // planet count range
};

static const StarTemplate STAR_TEMPLATES[STAR_CLASS_COUNT] = {
    // [0] O — Blue supergiant
    { 30000.0f, 1000000.0f,  30000.0f, 50000.0f,  6.6f, 15.0f,   1,  4 },
    // [1] B — Blue-white giant
    { 25.0f,    30000.0f,    10000.0f, 30000.0f,  1.8f,  6.6f,   2,  6 },
    // [2] A — White main-sequence
    { 5.0f,     25.0f,        7500.0f, 10000.0f,  1.4f,  1.8f,   3,  8 },
    // [3] F — Yellow-white
    { 1.5f,     5.0f,         6000.0f,  7500.0f,  1.15f, 1.4f,   4, 10 },
    // [4] G — Yellow (Sol-like)
    { 0.6f,     1.5f,         5200.0f,  6000.0f,  0.96f, 1.15f,  5, 12 },
    // [5] K — Orange dwarf
    { 0.08f,    0.6f,         3700.0f,  5200.0f,  0.7f,  0.96f,  4, 10 },
    // [6] M — Red dwarf
    { 0.001f,   0.08f,        2400.0f,  3700.0f,  0.1f,  0.7f,   3,  8 },
};

// ── Faction names for stations ─────────────────────────────────────
static const char* FACTIONS[] = {
    "Caldari Navy",    "Gallente Federation", "Amarr Empire",
    "Minmatar Republic", "ORE Consortium",    "Sisters of EVE",
    "Mordu's Legion",  "Serpentis Corp",      "Angel Cartel",
    "Guristas Pirates",
};
static constexpr int FACTION_COUNT = 10;

// ── Constants ──────────────────────────────────────────────────────
static constexpr float BASE_ORBIT_AU   = 0.3f;   // innermost orbit
static constexpr float ORBIT_SPACING   = 1.618f;  // golden ratio spacing factor
static constexpr int   MAX_BELTS       = 4;

// ── Public API ─────────────────────────────────────────────────────

GeneratedStarSystem StarSystemGenerator::generate(const PCGContext& ctx,
                                                   float securityLevel) {
    DeterministicRNG rng(ctx.seed);
    StarClass sc = selectStarClass(rng);
    return generate(ctx, securityLevel, sc);
}

GeneratedStarSystem StarSystemGenerator::generate(const PCGContext& ctx,
                                                   float securityLevel,
                                                   StarClass starOverride) {
    DeterministicRNG rng(ctx.seed);
    // Consume the same first roll to keep RNG state consistent.
    (void)rng.nextFloat();

    GeneratedStarSystem sys{};
    sys.systemId      = ctx.seed;
    sys.seed          = ctx.seed;
    sys.securityLevel = securityLevel;

    // Derive constellation ID from seed.
    sys.constellationId = deriveSeed(ctx.seed, 0xC005ULL);

    // ── Star properties ────────────────────────────────────────────
    const auto& st     = STAR_TEMPLATES[static_cast<int>(starOverride)];
    sys.star.starClass   = starOverride;
    sys.star.luminosity  = rng.rangeFloat(st.lumMin, st.lumMax);
    sys.star.temperature = rng.rangeFloat(st.tempMin, st.tempMax);
    sys.star.radius      = rng.rangeFloat(st.radiusMin, st.radiusMax);

    // ── Orbits, stations, gates ────────────────────────────────────
    generateOrbits(rng, sys, starOverride);
    placeStations(rng, sys);
    placeGates(rng, sys);

    // ── Tallies ────────────────────────────────────────────────────
    sys.totalPlanets = 0;
    sys.totalBelts   = 0;
    for (const auto& slot : sys.orbitSlots) {
        if (slot.type == OrbitSlotType::Planet) ++sys.totalPlanets;
        if (slot.type == OrbitSlotType::Belt)   ++sys.totalBelts;
    }

    sys.valid = (sys.totalPlanets > 0) && (sys.star.temperature > 0.0f);
    return sys;
}

std::string StarSystemGenerator::starClassName(StarClass sc) {
    switch (sc) {
        case StarClass::O: return "O";
        case StarClass::B: return "B";
        case StarClass::A: return "A";
        case StarClass::F: return "F";
        case StarClass::G: return "G";
        case StarClass::K: return "K";
        case StarClass::M: return "M";
    }
    return "Unknown";
}

// ── Internals ──────────────────────────────────────────────────────

StarClass StarSystemGenerator::selectStarClass(DeterministicRNG& rng) {
    // Weighted selection — realistic stellar distribution.
    //   M: 76%, K: 12%, G: 7.5%, F: 3%, A: 0.6%, B: 0.13%, O: 0.00003%
    // Compressed slightly so rarer stars appear more often in gameplay.
    //   M: 40%, K: 22%, G: 18%, F: 10%, A: 6%, B: 3%, O: 1%
    float roll = rng.nextFloat();
    float cum  = 0.0f;
    cum += 0.01f; if (roll < cum) return StarClass::O;
    cum += 0.03f; if (roll < cum) return StarClass::B;
    cum += 0.06f; if (roll < cum) return StarClass::A;
    cum += 0.10f; if (roll < cum) return StarClass::F;
    cum += 0.18f; if (roll < cum) return StarClass::G;
    cum += 0.22f; if (roll < cum) return StarClass::K;
    return StarClass::M;  // 40% — most common
}

void StarSystemGenerator::generateOrbits(DeterministicRNG& rng,
                                          GeneratedStarSystem& sys,
                                          StarClass sc) {
    const auto& st = STAR_TEMPLATES[static_cast<int>(sc)];
    int planetCount = rng.range(st.orbitMin, st.orbitMax);

    // Determine asteroid belt count (0–MAX_BELTS).
    int beltCount = rng.range(0, MAX_BELTS);

    int totalSlots = planetCount + beltCount;
    sys.orbitSlots.reserve(static_cast<size_t>(totalSlots));

    // Generate orbit radii using logarithmic spacing (Titius-Bode-like).
    float baseAU = BASE_ORBIT_AU + rng.rangeFloat(0.0f, 0.2f);

    // Build a merged list of slot indices; belts are randomly placed.
    // First, decide which slot indices are belts.
    if (beltCount > totalSlots) beltCount = totalSlots;
    std::vector<bool> isBelt(static_cast<size_t>(totalSlots), false);
    int beltPlaced    = 0;
    int max_attempts  = totalSlots * 4;  // prevent infinite loop on collisions
    int attempts      = 0;
    while (beltPlaced < beltCount && attempts < max_attempts) {
        int idx = rng.range(0, totalSlots - 1);
        if (!isBelt[static_cast<size_t>(idx)]) {
            isBelt[static_cast<size_t>(idx)] = true;
            ++beltPlaced;
        }
        ++attempts;
    }

    for (int i = 0; i < totalSlots; ++i) {
        OrbitSlot slot{};
        slot.orbitIndex  = i;
        // Logarithmic spacing: r = base * spacing^i with small jitter.
        slot.orbitRadius = baseAU * std::pow(ORBIT_SPACING, static_cast<float>(i));
        slot.orbitRadius *= rng.rangeFloat(0.9f, 1.1f);  // ±10% jitter

        if (isBelt[static_cast<size_t>(i)]) {
            slot.type       = OrbitSlotType::Belt;
            slot.planetType = SystemPlanetType::Barren;  // unused for belts
        } else {
            slot.type = OrbitSlotType::Planet;

            // Planet-type zoning based on orbit distance.
            float r = slot.orbitRadius;
            float roll = rng.nextFloat();

            if (r < 0.8f) {
                // Inner zone: Lava / Barren
                slot.planetType = (roll < 0.6f) ? SystemPlanetType::Lava
                                                 : SystemPlanetType::Barren;
            } else if (r < 3.0f) {
                // Habitable zone: Temperate / Oceanic / Barren
                if (roll < 0.40f)      slot.planetType = SystemPlanetType::Temperate;
                else if (roll < 0.70f) slot.planetType = SystemPlanetType::Oceanic;
                else                   slot.planetType = SystemPlanetType::Barren;
            } else if (r < 10.0f) {
                // Mid-outer zone: Gas / Storm
                if (roll < 0.55f)      slot.planetType = SystemPlanetType::Gas;
                else if (roll < 0.85f) slot.planetType = SystemPlanetType::Storm;
                else                   slot.planetType = SystemPlanetType::Ice;
            } else {
                // Outer zone: Ice / Gas
                if (roll < 0.60f)      slot.planetType = SystemPlanetType::Ice;
                else if (roll < 0.90f) slot.planetType = SystemPlanetType::Gas;
                else                   slot.planetType = SystemPlanetType::Storm;
            }
        }

        sys.orbitSlots.push_back(slot);
    }
}

void StarSystemGenerator::placeStations(DeterministicRNG& rng,
                                         GeneratedStarSystem& sys) {
    // Station count depends on security level.
    //   High-sec (≥0.5): 1–3 stations
    //   Low-sec  (0.1–0.5): 0–2 stations
    //   Null-sec (<0.1): 0–1 stations
    int minStations, maxStations;
    if (sys.securityLevel >= 0.5f) {
        minStations = 1;
        maxStations = 3;
    } else if (sys.securityLevel > 0.1f) {
        minStations = 0;
        maxStations = 2;
    } else {
        minStations = 0;
        maxStations = 1;
    }

    int stationCount = rng.range(minStations, maxStations);
    sys.stations.reserve(static_cast<size_t>(stationCount));

    int slotCount = static_cast<int>(sys.orbitSlots.size());
    if (slotCount == 0) return;

    for (int i = 0; i < stationCount; ++i) {
        SystemStation station{};
        station.stationId  = deriveSeed(sys.seed, static_cast<uint64_t>(0x57A0 + i));
        station.orbitIndex = rng.range(0, slotCount - 1);
        station.faction    = FACTIONS[rng.range(0, FACTION_COUNT - 1)];
        sys.stations.push_back(station);
    }
}

void StarSystemGenerator::placeGates(DeterministicRNG& rng,
                                      GeneratedStarSystem& sys) {
    // Gate count depends on security level.
    //   High-sec: 2–4 gates (well-connected)
    //   Low-sec:  1–3 gates
    //   Null-sec: 1–2 gates (sparse)
    int minGates, maxGates;
    if (sys.securityLevel >= 0.5f) {
        minGates = 2;
        maxGates = 4;
    } else if (sys.securityLevel > 0.1f) {
        minGates = 1;
        maxGates = 3;
    } else {
        minGates = 1;
        maxGates = 2;
    }

    int gateCount = rng.range(minGates, maxGates);
    sys.gates.reserve(static_cast<size_t>(gateCount));

    int slotCount = static_cast<int>(sys.orbitSlots.size());
    if (slotCount == 0) return;

    for (int i = 0; i < gateCount; ++i) {
        JumpGate gate{};
        gate.gateId              = deriveSeed(sys.seed, static_cast<uint64_t>(0x6A7E + i));
        gate.destinationSystemId = deriveSeed(sys.seed, static_cast<uint64_t>(0xDE57 + i));
        gate.orbitIndex          = rng.range(0, slotCount - 1);
        sys.gates.push_back(gate);
    }
}

} // namespace pcg
} // namespace atlas
