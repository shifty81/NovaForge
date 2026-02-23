#include "pcg/station_generator.h"
#include "pcg/hash_utils.h"
#include <cmath>

namespace atlas {
namespace pcg {

// ── Module templates (dims, power, attachment count) ───────────────

struct ModuleTemplate {
    float dimXMin, dimXMax;
    float dimYMin, dimYMax;
    float dimZMin, dimZMax;
    int   powerConsMin, powerConsMax;
    int   powerProdMin, powerProdMax;
    int   attachMin, attachMax;
};

static const ModuleTemplate MODULE_TEMPLATES[] = {
    // Habitat
    { 20, 40,  20, 40,  20, 40,   30, 60,    0,  0,   2, 4 },
    // Lab
    { 15, 30,  15, 30,  15, 30,   50, 100,   0,  0,   2, 3 },
    // DockingBay
    { 40, 80,  30, 60,  30, 60,   40, 80,    0,  0,   2, 4 },
    // Hangar
    { 50, 100, 40, 80,  40, 80,   60, 120,   0,  0,   3, 5 },
    // Corridor
    { 5,  15,  5,  15,  5,  15,   5,  10,    0,  0,   2, 2 },
    // Power
    { 25, 50,  25, 50,  25, 50,   10, 20,  200, 500,  3, 6 },
    // Storage
    { 20, 50,  20, 50,  20, 50,   10, 25,    0,  0,   2, 3 },
};

// ── Attachment-point generation ────────────────────────────────────

static std::vector<AttachmentPoint> generateAttachments(
        DeterministicRNG& rng, int count,
        float dx, float dy, float dz)
{
    // Six canonical face normals.
    static const float NORMALS[][3] = {
        { 1, 0, 0}, {-1, 0, 0},
        { 0, 1, 0}, { 0,-1, 0},
        { 0, 0, 1}, { 0, 0,-1},
    };

    std::vector<AttachmentPoint> pts;
    pts.reserve(static_cast<size_t>(count));
    for (int i = 0; i < count; ++i) {
        int face = rng.range(0, 5);
        AttachmentPoint ap{};
        ap.normalX = NORMALS[face][0];
        ap.normalY = NORMALS[face][1];
        ap.normalZ = NORMALS[face][2];
        // Place the point on the face centre offset by half-dimension.
        ap.posX = ap.normalX * dx * 0.5f;
        ap.posY = ap.normalY * dy * 0.5f;
        ap.posZ = ap.normalZ * dz * 0.5f;
        ap.occupied = false;
        pts.push_back(ap);
    }
    return pts;
}

// ── Public API ─────────────────────────────────────────────────────

GeneratedStation StationGenerator::generate(const PCGContext& ctx) {
    DeterministicRNG rng(ctx.seed);
    int moduleCount = rng.range(5, 15);
    return generate(ctx, moduleCount);
}

GeneratedStation StationGenerator::generate(const PCGContext& ctx,
                                            int moduleCount) {
    DeterministicRNG rng(ctx.seed);
    // Consume the same first roll so RNG state is consistent with the
    // single-arg overload when moduleCount happens to equal the roll.
    (void)rng.range(5, 15);

    GeneratedStation station{};
    station.stationId = ctx.seed;
    station.gravity   = rng.rangeFloat(0.1f, 1.0f);
    station.valid     = false;

    // First module is always a Power core at the origin.
    StationModule core = createModule(rng, 0, StationModuleType::Power);
    core.posX = 0.0f;
    core.posY = 0.0f;
    core.posZ = 0.0f;
    station.modules.push_back(core);

    // Generate remaining modules and snap to free attachment points.
    for (int i = 1; i < moduleCount; ++i) {
        StationModuleType type = selectModuleType(rng);
        StationModule mod = createModule(rng, i, type);
        snapModule(station, mod);
    }

    recalculatePower(station);
    station.valid = (station.totalPowerProduction >=
                     station.totalPowerConsumption)
                 && (static_cast<int>(station.modules.size()) >= 3);
    return station;
}

bool StationGenerator::snapModule(GeneratedStation& station,
                                  const StationModule& newModule) {
    // Find the first free attachment point across existing modules.
    for (auto& existing : station.modules) {
        for (auto& ap : existing.attachments) {
            if (ap.occupied) continue;

            ap.occupied = true;

            StationModule placed = newModule;
            // Position = parent world pos + attachment offset + outward
            // shift of half the new module's dimension along the normal.
            // Half-dimension along the attachment normal direction.
            float halfShift = (std::abs(ap.normalX) * placed.dimX
                             + std::abs(ap.normalY) * placed.dimY
                             + std::abs(ap.normalZ) * placed.dimZ) * 0.5f;
            placed.posX = existing.posX + ap.posX + ap.normalX * halfShift;
            placed.posY = existing.posY + ap.posY + ap.normalY * halfShift;
            placed.posZ = existing.posZ + ap.posZ + ap.normalZ * halfShift;

            station.modules.push_back(placed);
            return true;
        }
    }
    return false;
}

void StationGenerator::recalculatePower(GeneratedStation& station) {
    int totalCons = 0;
    int totalProd = 0;
    for (const auto& m : station.modules) {
        totalCons += m.powerConsumption;
        totalProd += m.powerProduction;
    }
    station.totalPowerConsumption = totalCons;
    station.totalPowerProduction  = totalProd;
}

// ── Internals ──────────────────────────────────────────────────────

StationModule StationGenerator::createModule(DeterministicRNG& rng,
                                             int id,
                                             StationModuleType type) {
    const auto& t = MODULE_TEMPLATES[static_cast<int>(type)];

    StationModule m{};
    m.moduleId         = id;
    m.type             = type;
    m.dimX             = rng.rangeFloat(t.dimXMin, t.dimXMax);
    m.dimY             = rng.rangeFloat(t.dimYMin, t.dimYMax);
    m.dimZ             = rng.rangeFloat(t.dimZMin, t.dimZMax);
    m.posX             = 0.0f;
    m.posY             = 0.0f;
    m.posZ             = 0.0f;
    m.powerConsumption = rng.range(t.powerConsMin, t.powerConsMax);
    m.powerProduction  = rng.range(t.powerProdMin, t.powerProdMax);

    int attachCount    = rng.range(t.attachMin, t.attachMax);
    m.attachments      = generateAttachments(rng, attachCount,
                                             m.dimX, m.dimY, m.dimZ);
    return m;
}

StationModuleType StationGenerator::selectModuleType(DeterministicRNG& rng) {
    // Weighted: Corridor 25%, Habitat 20%, Storage 15%, Lab 12%,
    //           DockingBay 10%, Hangar 10%, Power 8%
    float roll = rng.nextFloat();
    if (roll < 0.25f) return StationModuleType::Corridor;
    if (roll < 0.45f) return StationModuleType::Habitat;
    if (roll < 0.60f) return StationModuleType::Storage;
    if (roll < 0.72f) return StationModuleType::Lab;
    if (roll < 0.82f) return StationModuleType::DockingBay;
    if (roll < 0.92f) return StationModuleType::Hangar;
    return StationModuleType::Power;
}

} // namespace pcg
} // namespace atlas
