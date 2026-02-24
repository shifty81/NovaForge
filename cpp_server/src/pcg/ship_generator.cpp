#include "pcg/ship_generator.h"
#include "pcg/constraint_solver.h"
#include "pcg/pcg_trace.h"
#include <iterator>

namespace atlas {
namespace pcg {

// ── Hull-class base stats (mass, powergrid, cpu, weapon-size) ──────
struct HullTemplate {
    float massMin, massMax;         // tonnes
    float pgMin, pgMax;             // MW
    float cpuMin, cpuMax;           // tf
    float capMin, capMax;           // GJ
    int   turretMin, turretMax;
    int   launcherMin, launcherMax;
    WeaponSize maxWeapon;
    float armorMin, armorMax;
    float shieldMin, shieldMax;
    float sigMin, sigMax;           // signature radius metres
    float targetSpeedMin, targetSpeedMax; // seconds to lock
    int   droneBayMin, droneBayMax; // m³
    int   cargoMin, cargoMax;       // m³ cargo hold
};

// Array indexed by static_cast<int>(HullClass).
// Indices 0-5 match original classes for backward compatibility.
static const HullTemplate TEMPLATES[HULL_CLASS_COUNT] = {
    // [0] Frigate
    { 1'000, 2'500,    30, 55,    120, 200,    250, 400,   2, 3,  1, 2,  WeaponSize::Small,
      300, 600,   250, 500,   30, 45,    3.0f, 5.0f,    0, 25,   100, 400 },
    // [1] Destroyer
    { 2'000, 4'000,    45, 80,    170, 280,    350, 550,   4, 6,  2, 3,  WeaponSize::Small,
      600, 1200,  400, 800,   50, 75,    4.0f, 6.0f,    0, 25,   200, 600 },
    // [2] Cruiser
    { 8'000, 14'000,   700, 1200, 280, 400,    800, 1300,  3, 5,  2, 4,  WeaponSize::Medium,
      1800, 3500, 1500, 3000, 100, 150,  5.0f, 8.0f,    25, 75,  300, 900 },
    // [3] Battlecruiser
    { 12'000, 22'000,  1000, 1600, 350, 500,   1200, 1800, 5, 7,  3, 5,  WeaponSize::Medium,
      3000, 5500, 2500, 5000, 200, 280,  6.0f, 10.0f,   25, 75,  400, 1200 },
    // [4] Battleship
    { 80'000, 120'000, 8000, 14000, 500, 700,  4000, 6500, 6, 8,  4, 6,  WeaponSize::Large,
      6000, 10000, 5000, 9000, 300, 450,  8.0f, 14.0f,   50, 125, 600, 2000 },
    // [5] Capital (generic)
    { 800'000, 1'500'000, 60000, 120000, 700, 1000, 50000, 90000, 0, 2, 2, 6, WeaponSize::Large,
      40000, 80000, 30000, 60000, 2000, 5000, 14.0f, 25.0f, 100, 300, 2000, 8000 },

    // ── Tech II frigate variants ────────────────────────────────────
    // [6] Interceptor – T2 frigate, fast tackle, tiny sig
    { 800, 1'800,   25, 45,   130, 220,   200, 350,   2, 3,  0, 1,  WeaponSize::Small,
      200, 450,  200, 400,  25, 35,  2.0f, 4.0f,  0, 0,   80, 250 },
    // [7] CovertOps – T2 frigate, cloaky scout, high CPU
    { 900, 2'000,   20, 40,   180, 300,   200, 350,   1, 2,  0, 1,  WeaponSize::Small,
      200, 400,  180, 350,  20, 30,  2.5f, 4.5f,  0, 5,   100, 300 },
    // [8] AssaultFrigate – T2 frigate, heavy combat
    { 1'200, 2'800,  40, 65,   140, 240,   280, 450,   3, 4,  1, 2,  WeaponSize::Small,
      500, 900,  400, 750,  30, 45,  3.0f, 5.0f,  0, 15,  80, 300 },
    // [9] StealthBomber – T2 frigate, torpedo bomber (Large launchers)
    { 800, 2'000,   20, 35,   100, 180,   200, 350,   0, 1,  3, 4,  WeaponSize::Large,
      150, 350,  150, 300,  20, 30,  3.5f, 5.5f,  0, 0,   100, 300 },

    // ── Tech II cruiser variants ────────────────────────────────────
    // [10] Logistics – T2 cruiser, remote repair, high capacitor
    { 9'000, 15'000,  500, 900,  320, 480,   1200, 2000,  0, 2,  0, 1,  WeaponSize::Medium,
      2000, 4000,  1800, 3500,  90, 140,  5.0f, 8.0f,  25, 50,  200, 600 },
    // [11] Recon – T2 cruiser, ewar, high CPU
    { 8'000, 13'000,  600, 1000, 380, 550,   900, 1500,  2, 3,  1, 2,  WeaponSize::Medium,
      1500, 3000,  1200, 2500,  90, 130,  4.5f, 7.5f,  15, 50,  250, 700 },

    // ── Tech II battlecruiser variants ──────────────────────────────
    // [12] CommandShip – T2 battlecruiser, fleet boosts, heavy tank
    { 14'000, 25'000, 1200, 1800, 400, 600, 1400, 2200, 5, 7, 2, 4, WeaponSize::Medium,
      4000, 7000,  3500, 6500,  220, 300,  6.0f, 10.0f,  30, 75,  400, 1000 },

    // ── Tech II battleship variants ─────────────────────────────────
    // [13] Marauder – T2 battleship, bastion mode, very heavy
    { 90'000, 140'000, 10000, 16000, 550, 750, 5000, 8000, 4, 4, 4, 6, WeaponSize::Large,
      8000, 14000,  7000, 12000,  350, 500,  9.0f, 15.0f,  50, 100,  700, 2500 },

    // ── Industrial ──────────────────────────────────────────────────
    // [14] Industrial – hauler, huge cargo, minimal combat
    { 5'000, 20'000,  20, 50,   80, 150,   300, 600,   0, 1,  0, 0,  WeaponSize::Small,
      500, 1500,  400, 1200,  150, 300,  6.0f, 12.0f,  0, 0,   5000, 25000 },
    // [15] MiningBarge – mining focus, moderate
    { 8'000, 15'000,  30, 60,   100, 200,  500, 900,   0, 1,  0, 0,  WeaponSize::Small,
      800, 2000,  700, 1800,  120, 200,  6.0f, 10.0f,  25, 50,   3000, 12000 },
    // [16] Exhumer – T2 mining barge, improved yield + tank
    { 10'000, 18'000, 40, 80,  120, 250,  600, 1100,  0, 1,  0, 0,  WeaponSize::Small,
      1000, 2500,  900, 2200,  130, 220,  5.5f, 9.5f,  30, 60,   4000, 15000 },

    // ── Capital (specific) ──────────────────────────────────────────
    // [17] Carrier – fighter platform, massive drone bay
    { 500'000, 1'200'000, 40000, 80000, 600, 900, 40000, 75000, 0, 0, 0, 0, WeaponSize::Large,
      35000, 70000,  28000, 55000,  1800, 4500,  12.0f, 22.0f,  200, 500,  1500, 6000 },
    // [18] Dreadnought – siege, XLarge weapons, very heavy
    { 600'000, 1'500'000, 80000, 150000, 650, 950, 45000, 85000, 2, 3, 4, 6, WeaponSize::XLarge,
      45000, 90000,  35000, 70000,  2200, 5500,  14.0f, 25.0f,  0, 50,   2000, 8000 },
    // [19] Titan – super-capital, doomsday, XLarge weapons
    { 1'500'000, 3'000'000, 150000, 300000, 800, 1200, 80000, 150000, 3, 6, 6, 10, WeaponSize::XLarge,
      80000, 150000,  60000, 120000,  5000, 12000,  18.0f, 30.0f,  100, 300,  3000, 12000 },
};

// ── Static members ─────────────────────────────────────────────────

PCGTraceRecorder* ShipGenerator::traceRecorder_ = nullptr;

void ShipGenerator::setTraceRecorder(PCGTraceRecorder* recorder) {
    traceRecorder_ = recorder;
}

std::string ShipGenerator::hullClassName(HullClass hull) {
    switch (hull) {
        case HullClass::Frigate:       return "Frigate";
        case HullClass::Destroyer:     return "Destroyer";
        case HullClass::Cruiser:       return "Cruiser";
        case HullClass::Battlecruiser: return "Battlecruiser";
        case HullClass::Battleship:    return "Battleship";
        case HullClass::Capital:       return "Capital";
        case HullClass::Interceptor:   return "Interceptor";
        case HullClass::CovertOps:     return "CovertOps";
        case HullClass::AssaultFrigate:return "AssaultFrigate";
        case HullClass::StealthBomber: return "StealthBomber";
        case HullClass::Logistics:     return "Logistics";
        case HullClass::Recon:         return "Recon";
        case HullClass::CommandShip:   return "CommandShip";
        case HullClass::Marauder:      return "Marauder";
        case HullClass::Industrial:    return "Industrial";
        case HullClass::MiningBarge:   return "MiningBarge";
        case HullClass::Exhumer:       return "Exhumer";
        case HullClass::Carrier:       return "Carrier";
        case HullClass::Dreadnought:   return "Dreadnought";
        case HullClass::Titan:         return "Titan";
    }
    return "Unknown";
}

// ── Ship name tables (faction-neutral, procedural) ─────────────────
static const char* NAME_PREFIX[] = {
    "Tempest", "Vortex", "Zenith", "Eclipse", "Nova",
    "Rift", "Aegis", "Phantom", "Crucible", "Apex",
    "Wraith", "Bastion", "Specter", "Herald", "Sigil",
    "Torrent", "Cipher", "Vigil", "Talon", "Meridian",
};
static constexpr int NAME_PREFIX_COUNT = static_cast<int>(std::size(NAME_PREFIX));

static const char* NAME_SUFFIX[] = {
    "Mark", "Class", "Prime", "Alpha", "Vanguard",
    "Delta", "Core", "Edge", "Forge", "Crest",
};
static constexpr int NAME_SUFFIX_COUNT = static_cast<int>(std::size(NAME_SUFFIX));

std::string ShipGenerator::generateName(DeterministicRNG& rng, HullClass hull) {
    int prefixIdx = rng.range(0, NAME_PREFIX_COUNT - 1);
    int suffixIdx = rng.range(0, NAME_SUFFIX_COUNT - 1);
    int serial    = rng.range(1, 999);
    return std::string(NAME_PREFIX[prefixIdx]) + " " +
           std::string(NAME_SUFFIX[suffixIdx]) + "-" +
           std::to_string(serial);
}

// ── Public API ─────────────────────────────────────────────────────

GeneratedShip ShipGenerator::generate(const PCGContext& ctx) {
    DeterministicRNG rng(ctx.seed);
    GeneratedShip ship{};
    ship.shipId    = ctx.seed;
    ship.hullClass = selectHull(rng);
    deriveStats(rng, ship);
    attachEngines(rng, ship);
    attachWeapons(rng, ship);
    ship.shipName = generateName(rng, ship.hullClass);
    ship.valid = validateConstraints(ship);

    // Emit trace for debug visualization.
    if (traceRecorder_) {
        PCGTraceNode node{};
        node.seed     = ctx.seed;
        node.domain   = PCGDomain::Ship;
        node.objectId = ctx.seed;
        node.label    = "Ship";
        node.valid    = ship.valid;
        traceRecorder_->pushNode(node);
        traceRecorder_->annotate("Hull: " + hullClassName(ship.hullClass));
        traceRecorder_->annotate("Engines: " + std::to_string(ship.engineCount));
        traceRecorder_->annotate("Turrets: " + std::to_string(ship.turretSlots));
        traceRecorder_->annotate("Armor: " + std::to_string(static_cast<int>(ship.armorHP)));
        traceRecorder_->annotate("Shield: " + std::to_string(static_cast<int>(ship.shieldHP)));
        traceRecorder_->annotate("Name: " + ship.shipName);
        traceRecorder_->popNode();
    }

    return ship;
}

GeneratedShip ShipGenerator::generate(const PCGContext& ctx, HullClass hull) {
    DeterministicRNG rng(ctx.seed);
    GeneratedShip ship{};
    ship.shipId    = ctx.seed;
    ship.hullClass = hull;
    deriveStats(rng, ship);
    attachEngines(rng, ship);
    attachWeapons(rng, ship);
    ship.shipName = generateName(rng, ship.hullClass);
    ship.valid = validateConstraints(ship);
    return ship;
}

// ── Internals ──────────────────────────────────────────────────────

HullClass ShipGenerator::selectHull(DeterministicRNG& rng) {
    // Weighted selection — common ships are more likely.
    //  T1: Frigate 15%, Destroyer 12%, Cruiser 12%, BC 8%, BS 6%, Capital 2%
    //  T2 Frig: Interceptor 4%, CovertOps 3%, AssaultFrig 3%, StealthBomber 2%
    //  T2 Cruiser: Logistics 4%, Recon 3%
    //  T2 BC: CommandShip 3%
    //  T2 BS: Marauder 2%
    //  Industrial: Industrial 5%, MiningBarge 4%, Exhumer 2%
    //  Capital: Carrier 2%, Dreadnought 1.5%, Titan 0.5%
    //  (sums to ~92% — remaining goes to fallback Cruiser)
    float roll = rng.nextFloat();
    float cum = 0.0f;
    cum += 0.15f; if (roll < cum) return HullClass::Frigate;
    cum += 0.12f; if (roll < cum) return HullClass::Destroyer;
    cum += 0.12f; if (roll < cum) return HullClass::Cruiser;
    cum += 0.08f; if (roll < cum) return HullClass::Battlecruiser;
    cum += 0.06f; if (roll < cum) return HullClass::Battleship;
    cum += 0.02f; if (roll < cum) return HullClass::Capital;
    cum += 0.04f; if (roll < cum) return HullClass::Interceptor;
    cum += 0.03f; if (roll < cum) return HullClass::CovertOps;
    cum += 0.03f; if (roll < cum) return HullClass::AssaultFrigate;
    cum += 0.02f; if (roll < cum) return HullClass::StealthBomber;
    cum += 0.04f; if (roll < cum) return HullClass::Logistics;
    cum += 0.03f; if (roll < cum) return HullClass::Recon;
    cum += 0.03f; if (roll < cum) return HullClass::CommandShip;
    cum += 0.02f; if (roll < cum) return HullClass::Marauder;
    cum += 0.05f; if (roll < cum) return HullClass::Industrial;
    cum += 0.04f; if (roll < cum) return HullClass::MiningBarge;
    cum += 0.02f; if (roll < cum) return HullClass::Exhumer;
    cum += 0.02f; if (roll < cum) return HullClass::Carrier;
    cum += 0.015f; if (roll < cum) return HullClass::Dreadnought;
    cum += 0.005f; if (roll < cum) return HullClass::Titan;
    return HullClass::Cruiser;  // fallback for remaining probability
}

void ShipGenerator::deriveStats(DeterministicRNG& rng, GeneratedShip& ship) {
    const auto& t = TEMPLATES[static_cast<int>(ship.hullClass)];

    ship.mass       = rng.rangeFloat(t.massMin, t.massMax);
    ship.powergrid  = rng.rangeFloat(t.pgMin, t.pgMax);
    ship.cpu        = rng.rangeFloat(t.cpuMin, t.cpuMax);
    ship.capacitor  = rng.rangeFloat(t.capMin, t.capMax);
    ship.turretSlots   = rng.range(t.turretMin, t.turretMax);
    ship.launcherSlots = rng.range(t.launcherMin, t.launcherMax);
    ship.maxWeaponSize = t.maxWeapon;
    ship.armorHP         = rng.rangeFloat(t.armorMin, t.armorMax);
    ship.shieldHP        = rng.rangeFloat(t.shieldMin, t.shieldMax);
    ship.signatureRadius = rng.rangeFloat(t.sigMin, t.sigMax);
    ship.targetingSpeed  = rng.rangeFloat(t.targetSpeedMin, t.targetSpeedMax);
    ship.droneBay        = rng.range(t.droneBayMin, t.droneBayMax);
    ship.cargoCapacity   = rng.range(t.cargoMin, t.cargoMax);
    ship.techLevel       = hullTechLevel(ship.hullClass);
}

void ShipGenerator::attachEngines(DeterministicRNG& rng, GeneratedShip& ship) {
    // Engines must provide enough thrust so that align-time stays
    // within a reasonable band.  Desired accel ≥ 0.5 m/s².
    constexpr float MIN_ACCEL = 0.5f;
    float requiredThrust = ship.mass * MIN_ACCEL;

    // Engine count scales with hull class.
    int minEngines = 1;
    int maxEngines = 2;
    switch (ship.hullClass) {
        case HullClass::Frigate:       minEngines = 1; maxEngines = 2; break;
        case HullClass::Destroyer:     minEngines = 2; maxEngines = 3; break;
        case HullClass::Cruiser:       minEngines = 2; maxEngines = 4; break;
        case HullClass::Battlecruiser: minEngines = 3; maxEngines = 5; break;
        case HullClass::Battleship:    minEngines = 4; maxEngines = 6; break;
        case HullClass::Capital:       minEngines = 6; maxEngines = 10; break;
        // T2 frigate variants — same or slightly more than T1 frigate.
        case HullClass::Interceptor:   minEngines = 2; maxEngines = 3; break;
        case HullClass::CovertOps:     minEngines = 1; maxEngines = 2; break;
        case HullClass::AssaultFrigate:minEngines = 1; maxEngines = 3; break;
        case HullClass::StealthBomber: minEngines = 1; maxEngines = 2; break;
        // T2 cruiser variants.
        case HullClass::Logistics:     minEngines = 2; maxEngines = 4; break;
        case HullClass::Recon:         minEngines = 2; maxEngines = 4; break;
        // T2 battlecruiser.
        case HullClass::CommandShip:   minEngines = 3; maxEngines = 5; break;
        // T2 battleship.
        case HullClass::Marauder:      minEngines = 4; maxEngines = 6; break;
        // Industrial / mining — slow, few engines.
        case HullClass::Industrial:    minEngines = 1; maxEngines = 2; break;
        case HullClass::MiningBarge:   minEngines = 1; maxEngines = 2; break;
        case HullClass::Exhumer:       minEngines = 1; maxEngines = 3; break;
        // Capital (specific).
        case HullClass::Carrier:       minEngines = 6; maxEngines = 10; break;
        case HullClass::Dreadnought:   minEngines = 6; maxEngines = 10; break;
        case HullClass::Titan:         minEngines = 8; maxEngines = 14; break;
    }

    ship.engineCount = rng.range(minEngines, maxEngines);

    // Total thrust = per-engine thrust × count, always ≥ required.
    float perEngine = requiredThrust / static_cast<float>(ship.engineCount);
    perEngine *= rng.rangeFloat(1.0f, 1.5f); // 0-50% extra
    ship.thrust = perEngine * static_cast<float>(ship.engineCount);

    // Derive align time from the EVE formula approximation.
    ship.alignTime = ship.mass / ship.thrust;
}

// File-scope state for ConstraintSolver callbacks.
// NOTE: PCG generation is intentionally single-threaded (deterministic by
// design).  These statics exist because ConstraintSolver::MutateFn and
// FallbackFn are plain function pointers that cannot capture state.
static int*   s_turretSlots = nullptr;
static float  s_pgPerTurret = 0.0f;
static float* s_usedPg      = nullptr;
static PowerGridConstraint* s_pgConstraint = nullptr;

static void weaponMutator(DeterministicRNG& /*rng*/) {
    if (s_turretSlots && *s_turretSlots > 0) {
        (*s_turretSlots)--;
        if (s_usedPg) {
            *s_usedPg = s_pgPerTurret * static_cast<float>(*s_turretSlots);
            if (s_pgConstraint) s_pgConstraint->setUsedPower(*s_usedPg);
        }
    }
}

static void weaponFallback() {
    if (s_turretSlots) *s_turretSlots = 0;
    if (s_usedPg)      *s_usedPg = 0.0f;
    if (s_pgConstraint) s_pgConstraint->setUsedPower(0.0f);
}

void ShipGenerator::attachWeapons(DeterministicRNG& rng, GeneratedShip& ship) {
    float pgPerTurret = 0.0f;
    switch (ship.maxWeaponSize) {
        case WeaponSize::Small:  pgPerTurret = rng.rangeFloat(3.0f, 8.0f);   break;
        case WeaponSize::Medium: pgPerTurret = rng.rangeFloat(80.0f, 160.0f); break;
        case WeaponSize::Large:  pgPerTurret = rng.rangeFloat(1000.0f, 2500.0f); break;
        case WeaponSize::XLarge: pgPerTurret = rng.rangeFloat(5000.0f, 15000.0f); break;
    }

    float usedPg = pgPerTurret * static_cast<float>(ship.turretSlots);
    PowerGridConstraint pgConstraint(usedPg, ship.powergrid);

    // Wire file-scope state for solver callbacks.
    s_turretSlots  = &ship.turretSlots;
    s_pgPerTurret  = pgPerTurret;
    s_usedPg       = &usedPg;
    s_pgConstraint = &pgConstraint;

    ConstraintSolver solver;
    solver.add(&pgConstraint);
    solver.setMutator(weaponMutator);
    solver.setFallback(weaponFallback);
    solver.solve(rng);

    // Clear file-scope state.
    s_turretSlots  = nullptr;
    s_pgPerTurret  = 0.0f;
    s_usedPg       = nullptr;
    s_pgConstraint = nullptr;
}

bool ShipGenerator::validateConstraints(const GeneratedShip& ship) {
    if (ship.thrust <= 0.0f)    return false;
    if (ship.mass <= 0.0f)      return false;
    if (ship.alignTime <= 0.0f) return false;
    if (ship.capacitor <= 0.0f) return false;
    if (ship.armorHP <= 0.0f)   return false;
    if (ship.shieldHP <= 0.0f)  return false;
    if (ship.signatureRadius <= 0.0f) return false;
    if (ship.targetingSpeed <= 0.0f)  return false;
    return true;
}

} // namespace pcg
} // namespace atlas
