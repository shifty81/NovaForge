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
};

static const HullTemplate TEMPLATES[] = {
    // Frigate
    { 1'000, 2'500,    30, 55,    120, 200,    250, 400,   2, 3,  1, 2,  WeaponSize::Small,
      300, 600,   250, 500,   30, 45,    3.0f, 5.0f,    0, 25 },
    // Destroyer
    { 2'000, 4'000,    45, 80,    170, 280,    350, 550,   4, 6,  2, 3,  WeaponSize::Small,
      600, 1200,  400, 800,   50, 75,    4.0f, 6.0f,    0, 25 },
    // Cruiser
    { 8'000, 14'000,   700, 1200, 280, 400,    800, 1300,  3, 5,  2, 4,  WeaponSize::Medium,
      1800, 3500, 1500, 3000, 100, 150,  5.0f, 8.0f,    25, 75 },
    // Battlecruiser
    { 12'000, 22'000,  1000, 1600, 350, 500,   1200, 1800, 5, 7,  3, 5,  WeaponSize::Medium,
      3000, 5500, 2500, 5000, 200, 280,  6.0f, 10.0f,   25, 75 },
    // Battleship
    { 80'000, 120'000, 8000, 14000, 500, 700,  4000, 6500, 6, 8,  4, 6,  WeaponSize::Large,
      6000, 10000, 5000, 9000, 300, 450,  8.0f, 14.0f,   50, 125 },
    // Capital
    { 800'000, 1'500'000, 60000, 120000, 700, 1000, 50000, 90000, 0, 2, 2, 6, WeaponSize::Large,
      40000, 80000, 30000, 60000, 2000, 5000, 14.0f, 25.0f, 100, 300 },
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
    // Weighted selection: smaller hulls are more common.
    //  Frigate 30%, Destroyer 25%, Cruiser 20%,
    //  Battlecruiser 12%, Battleship 10%, Capital 3%
    float roll = rng.nextFloat();
    if (roll < 0.30f) return HullClass::Frigate;
    if (roll < 0.55f) return HullClass::Destroyer;
    if (roll < 0.75f) return HullClass::Cruiser;
    if (roll < 0.87f) return HullClass::Battlecruiser;
    if (roll < 0.97f) return HullClass::Battleship;
    return HullClass::Capital;
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
