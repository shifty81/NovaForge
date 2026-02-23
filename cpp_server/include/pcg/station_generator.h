#ifndef EVE_PCG_STATION_GENERATOR_H
#define EVE_PCG_STATION_GENERATOR_H

#include "pcg_context.h"
#include "deterministic_rng.h"
#include <vector>
#include <cstdint>

namespace atlas {
namespace pcg {

// ── Module types ────────────────────────────────────────────────────

enum class StationModuleType : uint32_t {
    Habitat,
    Lab,
    DockingBay,
    Hangar,
    Corridor,
    Power,
    Storage,
};

// ── Attachment point ────────────────────────────────────────────────

struct AttachmentPoint {
    float posX, posY, posZ;
    float normalX, normalY, normalZ;
    bool  occupied;
};

// ── Station module ──────────────────────────────────────────────────

struct StationModule {
    int               moduleId;
    StationModuleType type;
    float             dimX, dimY, dimZ;   ///< Dimensions (metres).
    float             posX, posY, posZ;   ///< World position.
    std::vector<AttachmentPoint> attachments;
    int               powerConsumption;
    int               powerProduction;
};

// ── Generated station ───────────────────────────────────────────────

struct GeneratedStation {
    uint64_t stationId;
    float    gravity;
    std::vector<StationModule> modules;
    int  totalPowerConsumption;
    int  totalPowerProduction;
    bool valid;               ///< true when power is sufficient and module count ≥ 3.
};

// ── Generator ───────────────────────────────────────────────────────

/**
 * @brief Deterministic modular station generator.
 *
 * Generates stations by placing a power core at the origin and
 * iteratively snapping additional modules to free attachment points.
 * The same PCGContext always produces the same station layout.
 */
class StationGenerator {
public:
    /** Generate a station with 5-15 modules (chosen procedurally). */
    static GeneratedStation generate(const PCGContext& ctx);

    /** Generate a station with a specific number of modules. */
    static GeneratedStation generate(const PCGContext& ctx, int moduleCount);

    /** Snap a module to the first free attachment point on the station. */
    static bool snapModule(GeneratedStation& station, const StationModule& newModule);

    /** Recalculate total power consumption / production. */
    static void recalculatePower(GeneratedStation& station);

private:
    static StationModule createModule(DeterministicRNG& rng, int id,
                                      StationModuleType type);
    static StationModuleType selectModuleType(DeterministicRNG& rng);
};

} // namespace pcg
} // namespace atlas

#endif // EVE_PCG_STATION_GENERATOR_H
