#include "pcg/rover_system.h"

namespace atlas {
namespace pcg {

static constexpr int   MIN_MODULES       = 2;
static constexpr int   MAX_MODULES       = 5;
static constexpr float BASE_MASS         = 500.0f;
static constexpr float MASS_PER_MODULE   = 100.0f;
static constexpr float BASE_SPEED        = 15.0f;
static constexpr float SPEED_MASS_FACTOR = 0.01f;

// ── Public API ─────────────────────────────────────────────────────

GeneratedRover RoverSystem::generate(const PCGContext& ctx) {
    DeterministicRNG rng(ctx.seed);
    int moduleCount = rng.range(MIN_MODULES, MAX_MODULES);
    return generate(ctx, moduleCount);
}

GeneratedRover RoverSystem::generate(const PCGContext& ctx, int moduleCount) {
    DeterministicRNG rng(ctx.seed);

    int count = (moduleCount < MIN_MODULES) ? MIN_MODULES
              : (moduleCount > MAX_MODULES) ? MAX_MODULES
              : moduleCount;

    GeneratedRover rover{};
    rover.roverId  = ctx.seed;
    rover.deployed = false;
    rover.posX     = 0.0f;
    rover.posY     = 0.0f;
    rover.posZ     = 0.0f;

    // First module is always Cargo
    rover.modules.push_back(generateModule(rng, RoverModuleType::Cargo));

    for (int i = 1; i < count; ++i) {
        RoverModuleType type = selectModuleType(rng);
        rover.modules.push_back(generateModule(rng, type));
    }

    rover.mass     = BASE_MASS + MASS_PER_MODULE * static_cast<float>(rover.modules.size());
    rover.maxSpeed = BASE_SPEED - rover.mass * SPEED_MASS_FACTOR;
    rover.cargoCapacity = totalCargoCapacity(rover);
    rover.valid    = rover.modules.size() >= static_cast<size_t>(MIN_MODULES);

    return rover;
}

bool RoverSystem::deploy(GeneratedRover& rover, float x, float y, float z) {
    if (rover.deployed) return false;
    rover.posX     = x;
    rover.posY     = y;
    rover.posZ     = z;
    rover.deployed = true;
    return true;
}

bool RoverSystem::dock(GeneratedRover& rover) {
    if (!rover.deployed) return false;
    rover.deployed = false;
    return true;
}

int RoverSystem::totalCargoCapacity(const GeneratedRover& rover) {
    int total = 0;
    for (const auto& mod : rover.modules) {
        if (mod.type == RoverModuleType::Cargo) {
            total += mod.capacity;
        }
    }
    return total;
}

// ── Internals ──────────────────────────────────────────────────────

RoverModuleType RoverSystem::selectModuleType(DeterministicRNG& rng) {
    // Weighted: 30% Cargo, 25% Scanner, 20% MiningLaser,
    //           15% RepairKit, 10% WeaponMount
    float roll = rng.nextFloat();
    if (roll < 0.30f) return RoverModuleType::Cargo;
    if (roll < 0.55f) return RoverModuleType::Scanner;
    if (roll < 0.75f) return RoverModuleType::MiningLaser;
    if (roll < 0.90f) return RoverModuleType::RepairKit;
    return RoverModuleType::WeaponMount;
}

RoverModule RoverSystem::generateModule(DeterministicRNG& rng, RoverModuleType type) {
    RoverModule mod{};
    mod.type = type;

    switch (type) {
        case RoverModuleType::Cargo:
            mod.capacity   = rng.range(50, 200);
            mod.efficiency = 1.0f;
            break;
        case RoverModuleType::MiningLaser:
            mod.capacity   = 0;
            mod.efficiency = rng.rangeFloat(0.5f, 1.5f);
            break;
        case RoverModuleType::Scanner:
            mod.capacity   = 0;
            mod.efficiency = rng.rangeFloat(0.6f, 1.2f);
            break;
        case RoverModuleType::RepairKit:
            mod.capacity   = 0;
            mod.efficiency = rng.rangeFloat(0.4f, 1.0f);
            break;
        case RoverModuleType::WeaponMount:
            mod.capacity   = 0;
            mod.efficiency = rng.rangeFloat(0.7f, 1.3f);
            break;
    }

    return mod;
}

} // namespace pcg
} // namespace atlas
