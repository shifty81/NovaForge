#ifndef EVE_PCG_ROVER_SYSTEM_H
#define EVE_PCG_ROVER_SYSTEM_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Rover module types ──────────────────────────────────────────────
enum class RoverModuleType : uint32_t {
    Cargo,
    MiningLaser,
    Scanner,
    RepairKit,
    WeaponMount,
};

// ── Rover module data ───────────────────────────────────────────────
struct RoverModule {
    RoverModuleType type;
    int capacity;      // for cargo modules
    float efficiency;  // for tools
};

// ── Generated rover data ────────────────────────────────────────────
struct GeneratedRover {
    uint64_t roverId;
    float maxSpeed;
    float mass;
    int cargoCapacity;
    std::vector<RoverModule> modules;
    bool deployed;
    float posX, posY, posZ;
    bool valid;
};

/**
 * @brief Planetary rover configuration and deployment.
 *
 * Given a PCGContext the system produces a deterministic rover
 * with procedurally generated modules and stats.
 */
class RoverSystem {
public:
    static GeneratedRover generate(const PCGContext& ctx);
    static GeneratedRover generate(const PCGContext& ctx, int moduleCount);
    static bool deploy(GeneratedRover& rover, float x, float y, float z);
    static bool dock(GeneratedRover& rover);
    static int totalCargoCapacity(const GeneratedRover& rover);

private:
    static RoverModuleType selectModuleType(DeterministicRNG& rng);
    static RoverModule generateModule(DeterministicRNG& rng, RoverModuleType type);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_ROVER_SYSTEM_H
