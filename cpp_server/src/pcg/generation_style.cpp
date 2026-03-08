#include "pcg/generation_style.h"
#include "pcg/snappable_grid.h"

#include <sstream>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Style-type names ───────────────────────────────────────────────

const char* GenerationStyleEngine::styleTypeName(GenerationStyleType type) {
    switch (type) {
        case GenerationStyleType::ShipLayout:       return "ShipLayout";
        case GenerationStyleType::StationLayout:    return "StationLayout";
        case GenerationStyleType::InteriorLayout:   return "InteriorLayout";
        case GenerationStyleType::StarSystem:       return "StarSystem";
        case GenerationStyleType::AsteroidField:    return "AsteroidField";
        case GenerationStyleType::FleetComposition: return "FleetComposition";
    }
    return "Unknown";
}

// ── Available parameters per style type ────────────────────────────

std::vector<std::string> GenerationStyleEngine::availableParameters(
        GenerationStyleType type) {
    switch (type) {
        case GenerationStyleType::ShipLayout:
            return { "hullClass", "techLevel", "turretSlots",
                     "launcherSlots", "engineCount", "massScale" };

        case GenerationStyleType::StationLayout:
            return { "moduleCount", "maxPowerRatio",
                     "minDockingBays", "habitatRatio" };

        case GenerationStyleType::InteriorLayout:
            return { "shipClass", "deckCount", "corridorWidth",
                     "roomBudgetScale" };

        case GenerationStyleType::StarSystem:
            return { "securityLevel", "planetCount", "beltCount",
                     "stationCount", "gateCount" };

        case GenerationStyleType::AsteroidField:
            return { "density", "richness", "fieldRadius",
                     "clusterCount" };

        case GenerationStyleType::FleetComposition:
            return { "fleetSize", "capitalRatio", "doctrineAggression",
                     "supportRatio" };
    }
    return {};
}

// ── Create default style ───────────────────────────────────────────

static ParameterOverride makeParam(const std::string& name,
                                   float value,
                                   float minVal,
                                   float maxVal) {
    return { name, value, minVal, maxVal, /*enabled=*/true };
}

GenerationStyle GenerationStyleEngine::createDefaultStyle(
        GenerationStyleType type, const std::string& name) {
    GenerationStyle style{};
    style.name     = name.empty() ? std::string("New ") + styleTypeName(type)
                                  : name;
    style.type     = type;
    style.version  = 1;
    style.baseSeed = 42;
    style.valid    = false;

    switch (type) {
        case GenerationStyleType::ShipLayout:
            style.parameters = {
                makeParam("hullClass",      0.0f,  0.0f, 19.0f),
                makeParam("techLevel",      1.0f,  1.0f,  2.0f),
                makeParam("turretSlots",    0.0f,  0.0f, 12.0f),
                makeParam("launcherSlots",  0.0f,  0.0f,  8.0f),
                makeParam("engineCount",    0.0f,  0.0f,  6.0f),
                makeParam("massScale",      1.0f,  0.5f,  3.0f),
            };
            break;

        case GenerationStyleType::StationLayout:
            style.parameters = {
                makeParam("moduleCount",    0.0f,  3.0f, 20.0f),
                makeParam("maxPowerRatio",  0.85f, 0.5f,  1.0f),
                makeParam("minDockingBays", 1.0f,  0.0f,  5.0f),
                makeParam("habitatRatio",   0.3f,  0.0f,  1.0f),
            };
            break;

        case GenerationStyleType::InteriorLayout:
            style.parameters = {
                makeParam("shipClass",      0.0f,  0.0f, 5.0f),
                makeParam("deckCount",      0.0f,  1.0f, 8.0f),
                makeParam("corridorWidth",  0.0f,  1.2f, 4.0f),
                makeParam("roomBudgetScale",1.0f,  0.5f, 2.0f),
            };
            break;

        case GenerationStyleType::StarSystem:
            style.parameters = {
                makeParam("securityLevel",  0.5f,  0.0f, 1.0f),
                makeParam("planetCount",    0.0f,  1.0f, 15.0f),
                makeParam("beltCount",      0.0f,  0.0f,  4.0f),
                makeParam("stationCount",   0.0f,  0.0f,  5.0f),
                makeParam("gateCount",      0.0f,  1.0f,  6.0f),
            };
            break;

        case GenerationStyleType::AsteroidField:
            style.parameters = {
                makeParam("density",        0.15f, 0.01f, 0.5f),
                makeParam("richness",       0.1f,  0.0f,  1.0f),
                makeParam("fieldRadius",    50.0f, 10.0f, 200.0f),
                makeParam("clusterCount",   3.0f,  1.0f,  10.0f),
            };
            break;

        case GenerationStyleType::FleetComposition:
            style.parameters = {
                makeParam("fleetSize",      5.0f,  1.0f, 50.0f),
                makeParam("capitalRatio",   0.0f,  0.0f,  0.5f),
                makeParam("doctrineAggression", 0.5f, 0.0f, 1.0f),
                makeParam("supportRatio",   0.2f,  0.0f,  0.5f),
            };
            break;
    }

    return style;
}

// ── Validate ───────────────────────────────────────────────────────

bool GenerationStyleEngine::validate(GenerationStyle& style) {
    // Name must not be empty.
    if (style.name.empty()) {
        style.valid = false;
        return false;
    }

    // Each enabled parameter must be within its range.
    for (auto& p : style.parameters) {
        if (!p.enabled) continue;
        if (p.value < p.minValue || p.value > p.maxValue) {
            style.valid = false;
            return false;
        }
    }

    // Placement slot indices must be unique.
    std::vector<uint32_t> slots;
    for (const auto& pe : style.placements) {
        if (std::find(slots.begin(), slots.end(), pe.slotIndex)
                != slots.end()) {
            style.valid = false;
            return false;
        }
        slots.push_back(pe.slotIndex);
    }

    style.valid = true;
    return true;
}

// ── Find parameter ─────────────────────────────────────────────────

const ParameterOverride* GenerationStyleEngine::findParameter(
        const GenerationStyle& style, const std::string& name) {
    for (const auto& p : style.parameters) {
        if (p.name == name) return &p;
    }
    return nullptr;
}

// ── Helper: read an enabled parameter value or return a default ────

static float paramOr(const GenerationStyle& style,
                     const std::string& name,
                     float defaultVal) {
    const ParameterOverride* p =
        GenerationStyleEngine::findParameter(style, name);
    if (p && p->enabled) return p->value;
    return defaultVal;
}

// ── Generate (dispatch) ────────────────────────────────────────────

StyleGenerationResult GenerationStyleEngine::generate(
        const PCGContext& ctx, const GenerationStyle& style) {
    switch (style.type) {
        case GenerationStyleType::ShipLayout:
            return generateShipLayout(ctx, style);
        case GenerationStyleType::StationLayout:
            return generateStationLayout(ctx, style);
        case GenerationStyleType::InteriorLayout:
            return generateInteriorLayout(ctx, style);
        case GenerationStyleType::StarSystem:
            return generateStarSystem(ctx, style);
        case GenerationStyleType::AsteroidField:
            return generateAsteroidField(ctx, style);
        case GenerationStyleType::FleetComposition:
            return generateFleetComposition(ctx, style);
    }

    StyleGenerationResult r{};
    r.success      = false;
    r.errorMessage = "Unknown style type";
    return r;
}

// ── Ship layout generation ─────────────────────────────────────────

StyleGenerationResult GenerationStyleEngine::generateShipLayout(
        const PCGContext& ctx, const GenerationStyle& style) {
    StyleGenerationResult result{};
    result.sourceType = style.type;
    result.styleName  = style.name;

    int paramsApplied = 0;

    // Determine hull class from parameter or let the generator decide.
    float hullVal = paramOr(style, "hullClass", -1.0f);
    bool overrideHull = (hullVal >= 0.0f && hullVal <= 19.0f);

    GeneratedShip ship;
    if (overrideHull) {
        ship = ShipGenerator::generate(
            ctx, static_cast<HullClass>(static_cast<int>(hullVal)));
        ++paramsApplied;
    } else {
        ship = ShipGenerator::generate(ctx);
    }

    // Apply parameter overrides.
    float massScale = paramOr(style, "massScale", 1.0f);
    if (std::fabs(massScale - 1.0f) > 0.001f) {
        ship.mass    *= massScale;
        ship.armorHP *= massScale;
        ++paramsApplied;
    }

    float turretOvr = paramOr(style, "turretSlots", 0.0f);
    if (turretOvr > 0.0f) {
        ship.turretSlots = static_cast<int>(turretOvr);
        ++paramsApplied;
    }

    float launcherOvr = paramOr(style, "launcherSlots", 0.0f);
    if (launcherOvr > 0.0f) {
        ship.launcherSlots = static_cast<int>(launcherOvr);
        ++paramsApplied;
    }

    float engineOvr = paramOr(style, "engineCount", 0.0f);
    if (engineOvr > 0.0f) {
        ship.engineCount = static_cast<int>(engineOvr);
        ++paramsApplied;
    }

    // Designer placements can pin turret/launcher positions.
    int placementsApplied = 0;
    for (const auto& pe : style.placements) {
        if (pe.locked) {
            // Locked placements are honoured verbatim.
            ++placementsApplied;
        }
    }

    result.shipResult        = ship;
    result.placementsApplied = placementsApplied;
    result.parametersApplied = paramsApplied;
    result.success           = ship.valid;
    if (!result.success) {
        result.errorMessage = "Ship failed validation";
    }
    return result;
}

// ── Station layout generation ──────────────────────────────────────

int GenerationStyleEngine::applyStationPlacements(
        GeneratedStation& station,
        const std::vector<PlacementEntry>& placements) {
    int applied = 0;
    for (const auto& pe : placements) {
        if (pe.slotIndex < station.modules.size()) {
            auto& mod = station.modules[pe.slotIndex];

            // Override module type from contentType.
            if (pe.contentType <= static_cast<uint32_t>(
                    StationModuleType::Storage)) {
                mod.type = static_cast<StationModuleType>(pe.contentType);
            }

            // Pin position if the designer specified one.
            if (pe.locked) {
                mod.posX = pe.posX;
                mod.posY = pe.posY;
                mod.posZ = pe.posZ;
            }

            if (!pe.label.empty()) {
                // Label stored for UI display (modules don't carry names
                // in the base struct, but the placement is honoured).
            }
            ++applied;
        }
    }
    return applied;
}

StyleGenerationResult GenerationStyleEngine::generateStationLayout(
        const PCGContext& ctx, const GenerationStyle& style) {
    StyleGenerationResult result{};
    result.sourceType = style.type;
    result.styleName  = style.name;

    int paramsApplied = 0;

    // Module count from parameter or generator default.
    float modCountVal = paramOr(style, "moduleCount", 0.0f);
    bool overrideCount = (modCountVal >= 3.0f);

    GeneratedStation station;
    if (overrideCount) {
        station = StationGenerator::generate(
            ctx, static_cast<int>(modCountVal));
        ++paramsApplied;
    } else {
        station = StationGenerator::generate(ctx);
    }

    // Apply designer placements.
    int placementsApplied = applyStationPlacements(
        station, style.placements);

    // Recalculate power after placement changes.
    StationGenerator::recalculatePower(station);

    // Check max power ratio constraint.
    float maxPowerRatio = paramOr(style, "maxPowerRatio", 1.0f);
    if (maxPowerRatio < 1.0f && station.totalPowerProduction > 0) {
        float ratio = static_cast<float>(station.totalPowerConsumption) /
                      static_cast<float>(station.totalPowerProduction);
        if (ratio > maxPowerRatio) {
            // Flag as invalid but still return the result.
            station.valid = false;
        }
        ++paramsApplied;
    }

    result.stationResult     = station;
    result.placementsApplied = placementsApplied;
    result.parametersApplied = paramsApplied;
    result.success           = station.valid;
    if (!result.success) {
        result.errorMessage = "Station failed validation";
    }
    return result;
}

// ── Interior layout generation ─────────────────────────────────────

StyleGenerationResult GenerationStyleEngine::generateInteriorLayout(
        const PCGContext& ctx, const GenerationStyle& style) {
    StyleGenerationResult result{};
    result.sourceType = style.type;
    result.styleName  = style.name;

    int paramsApplied = 0;

    int shipClass = static_cast<int>(paramOr(style, "shipClass", 0.0f));
    if (shipClass < 0) shipClass = 0;
    if (shipClass > 5) shipClass = 5;
    ++paramsApplied;

    GeneratedInterior interior = InteriorGenerator::generate(ctx, shipClass);

    // Apply placements — map to room overrides.
    int placementsApplied = 0;
    for (const auto& pe : style.placements) {
        if (pe.slotIndex < interior.rooms.size()) {
            auto& room = interior.rooms[pe.slotIndex];
            if (pe.contentType <=
                    static_cast<uint32_t>(InteriorRoomType::EscapePod)) {
                room.type = static_cast<InteriorRoomType>(pe.contentType);
            }
            if (pe.locked) {
                room.posX = pe.posX;
                room.posY = pe.posY;
                room.posZ = pe.posZ;
            }
            ++placementsApplied;
        }
    }

    result.interiorResult    = interior;
    result.placementsApplied = placementsApplied;
    result.parametersApplied = paramsApplied;
    result.success           = interior.valid;
    if (!result.success) {
        result.errorMessage = "Interior failed FPS validation";
    }
    return result;
}

// ── Star system generation ──────────────────────────────────────────

static const char* kStarNames[] = {
    "Helios", "Vega", "Altair", "Rigel", "Deneb",
    "Sirius", "Capella", "Antares", "Polaris", "Arcturus",
    "Betelgeuse", "Procyon", "Aldebaran", "Regulus", "Spica",
};
static constexpr int kStarNameCount = 15;

static const char* kPlanetSuffixes[] = {
    "I", "II", "III", "IV", "V", "VI", "VII", "VIII",
    "IX", "X", "XI", "XII", "XIII", "XIV", "XV",
};

StyleGenerationResult GenerationStyleEngine::generateStarSystem(
        const PCGContext& ctx, const GenerationStyle& style) {
    StyleGenerationResult result{};
    result.sourceType = style.type;
    result.styleName  = style.name;

    DeterministicRNG rng(ctx.seed);
    int paramsApplied = 0;

    float securityLevel = paramOr(style, "securityLevel", 0.5f);
    ++paramsApplied;

    float planetCountF = paramOr(style, "planetCount", 0.0f);
    int planetCount = (planetCountF >= 1.0f)
        ? static_cast<int>(planetCountF)
        : rng.range(3, 10);
    if (planetCountF >= 1.0f) ++paramsApplied;

    float beltCountF = paramOr(style, "beltCount", -1.0f);
    int beltCount = (beltCountF >= 0.0f)
        ? static_cast<int>(beltCountF)
        : rng.range(0, 3);
    if (beltCountF >= 0.0f) ++paramsApplied;

    float stationCountF = paramOr(style, "stationCount", -1.0f);
    int stationCount = (stationCountF >= 0.0f)
        ? static_cast<int>(stationCountF)
        : rng.range(0, 3);
    if (stationCountF >= 0.0f) ++paramsApplied;

    float gateCountF = paramOr(style, "gateCount", 0.0f);
    int gateCount = (gateCountF >= 1.0f)
        ? static_cast<int>(gateCountF)
        : rng.range(1, 4);
    if (gateCountF >= 1.0f) ++paramsApplied;

    GeneratedStarSystem sys{};
    sys.systemId       = rng.nextU32();
    sys.systemName     = kStarNames[rng.range(0, kStarNameCount - 1)];
    sys.securityLevel  = securityLevel;
    sys.starType       = rng.range(0, 3);
    sys.starLuminosity = rng.rangeFloat(0.5f, 5.0f);
    sys.beltCount      = beltCount;
    sys.stationCount   = stationCount;

    // Generate planets.
    for (int i = 0; i < planetCount; ++i) {
        GeneratedCelestialBody planet{};
        planet.bodyId      = static_cast<uint32_t>(i);
        planet.name        = sys.systemName + " " + kPlanetSuffixes[i % 15];
        planet.orbitRadius = 0.3f + static_cast<float>(i) * rng.rangeFloat(0.4f, 1.2f);
        planet.orbitAngle  = rng.rangeFloat(0.0f, 6.2831853f);
        planet.radius      = rng.rangeFloat(2000.0f, 70000.0f);
        planet.bodyType    = rng.range(0, 3);
        sys.planets.push_back(planet);
    }

    // Generate stargates.
    constexpr float kGateVerticalSpread = 2.0f;

    for (int i = 0; i < gateCount; ++i) {
        GeneratedStargate gate{};
        gate.gateId = static_cast<uint32_t>(i);
        gate.name   = sys.systemName + " Gate " + std::to_string(i + 1);
        float angle = rng.rangeFloat(0.0f, 6.2831853f);
        float dist  = rng.rangeFloat(10.0f, 50.0f);
        gate.posX   = dist * std::cos(angle);
        gate.posY   = rng.rangeFloat(-kGateVerticalSpread, kGateVerticalSpread);
        gate.posZ   = dist * std::sin(angle);
        sys.stargates.push_back(gate);
    }

    // Apply designer placements (pin specific celestials).
    int placementsApplied = 0;
    for (const auto& pe : style.placements) {
        if (pe.slotIndex < sys.planets.size()) {
            auto& planet = sys.planets[pe.slotIndex];
            if (pe.contentType <= 3) {
                planet.bodyType = pe.contentType;
            }
            if (pe.locked) {
                planet.orbitRadius = pe.posX;
                planet.orbitAngle  = pe.posY;
                planet.radius      = pe.posZ;
            }
            ++placementsApplied;
        }
    }

    sys.valid = (!sys.planets.empty() && !sys.stargates.empty());

    result.starSystemResult  = sys;
    result.placementsApplied = placementsApplied;
    result.parametersApplied = paramsApplied;
    result.success           = sys.valid;
    if (!result.success) {
        result.errorMessage = "Star system failed validation";
    }
    return result;
}

// ── Asteroid field generation ───────────────────────────────────────

static const char* kOreNames[] = {
    "Veldspar", "Scordite", "Pyroxeres", "Plagioclase",
    "Omber", "Kernite", "Dark Ochre", "Arkonor",
};

StyleGenerationResult GenerationStyleEngine::generateAsteroidField(
        const PCGContext& ctx, const GenerationStyle& style) {
    StyleGenerationResult result{};
    result.sourceType = style.type;
    result.styleName  = style.name;

    DeterministicRNG rng(ctx.seed);
    int paramsApplied = 0;

    float density = paramOr(style, "density", 0.15f);
    ++paramsApplied;

    float richness = paramOr(style, "richness", 0.1f);
    ++paramsApplied;

    float fieldRadius = paramOr(style, "fieldRadius", 50.0f);
    ++paramsApplied;

    float clusterCountF = paramOr(style, "clusterCount", 3.0f);
    int clusterCount = static_cast<int>(clusterCountF);
    if (clusterCount < 1) clusterCount = 1;
    ++paramsApplied;

    GeneratedAsteroidField field{};
    field.fieldId      = rng.nextU32();
    field.fieldName    = std::string("Belt-") + std::to_string(field.fieldId % 1000);
    field.centerX      = 0.0f;
    field.centerY      = 0.0f;
    field.centerZ      = 0.0f;
    field.fieldRadius  = fieldRadius;
    field.clusterCount = clusterCount;

    // Asteroid count scales linearly with density and field diameter.
    int asteroidCount = std::max(1, static_cast<int>(
        density * fieldRadius * 2.0f));

    // Layout constants: asteroid belts are disc-shaped (thin in Y).
    constexpr float kClusterRadialFactor   = 0.7f;
    constexpr float kClusterVerticalFactor = 0.1f;
    constexpr float kClusterSpreadFactor   = 0.2f;
    constexpr float kVerticalSpreadFactor  = 0.3f;

    uint32_t nextId = 0;
    for (int c = 0; c < clusterCount; ++c) {
        // Cluster centre within the field radius.
        float cx = rng.rangeFloat(-fieldRadius * kClusterRadialFactor,
                                   fieldRadius * kClusterRadialFactor);
        float cy = rng.rangeFloat(-fieldRadius * kClusterVerticalFactor,
                                   fieldRadius * kClusterVerticalFactor);
        float cz = rng.rangeFloat(-fieldRadius * kClusterRadialFactor,
                                   fieldRadius * kClusterRadialFactor);

        int perCluster = asteroidCount / clusterCount;
        for (int i = 0; i < perCluster; ++i) {
            GeneratedAsteroid ast{};
            ast.asteroidId = nextId++;
            float spread = fieldRadius * kClusterSpreadFactor;
            ast.posX = cx + rng.rangeFloat(-spread, spread);
            ast.posY = cy + rng.rangeFloat(-spread * kVerticalSpreadFactor,
                                            spread * kVerticalSpreadFactor);
            ast.posZ = cz + rng.rangeFloat(-spread, spread);
            ast.radius   = rng.rangeFloat(5.0f, 80.0f);
            ast.oreType  = rng.range(0, 7);
            ast.richness = richness * rng.rangeFloat(0.5f, 1.5f);
            field.asteroids.push_back(ast);
        }
    }

    // Apply designer placements.
    int placementsApplied = 0;
    for (const auto& pe : style.placements) {
        if (pe.slotIndex < field.asteroids.size()) {
            auto& ast = field.asteroids[pe.slotIndex];
            if (pe.contentType <= 7) {
                ast.oreType = pe.contentType;
            }
            if (pe.locked) {
                ast.posX = pe.posX;
                ast.posY = pe.posY;
                ast.posZ = pe.posZ;
            }
            ++placementsApplied;
        }
    }

    field.valid = !field.asteroids.empty();

    result.asteroidFieldResult = field;
    result.placementsApplied   = placementsApplied;
    result.parametersApplied   = paramsApplied;
    result.success             = field.valid;
    if (!result.success) {
        result.errorMessage = "Asteroid field failed validation";
    }
    return result;
}

// ── Fleet composition generation ────────────────────────────────────

static const char* kDoctrineNames[] = {
    "Wolfpack", "Shield Wall", "Hammer Fleet", "Skirmish Line",
    "Iron Curtain", "Storm Front", "Vanguard", "Rearguard",
};
static constexpr int kDoctrineNameCount = 8;

static const char* kFleetRoles[] = {
    "dps", "logistics", "ewar", "tackle", "command",
};

StyleGenerationResult GenerationStyleEngine::generateFleetComposition(
        const PCGContext& ctx, const GenerationStyle& style) {
    StyleGenerationResult result{};
    result.sourceType = style.type;
    result.styleName  = style.name;

    DeterministicRNG rng(ctx.seed);
    int paramsApplied = 0;

    float fleetSizeF = paramOr(style, "fleetSize", 5.0f);
    int fleetSize = std::max(1, static_cast<int>(fleetSizeF));
    ++paramsApplied;

    float capitalRatio = paramOr(style, "capitalRatio", 0.0f);
    ++paramsApplied;

    float aggression = paramOr(style, "doctrineAggression", 0.5f);
    ++paramsApplied;

    float supportRatio = paramOr(style, "supportRatio", 0.2f);
    ++paramsApplied;

    GeneratedFleetCompositionResult fleet{};
    fleet.fleetId         = rng.nextU32();
    fleet.doctrineName    = kDoctrineNames[rng.range(0, kDoctrineNameCount - 1)];
    fleet.aggressionRating = aggression;

    int capitalCount = static_cast<int>(
        static_cast<float>(fleetSize) * capitalRatio);
    int subcapCount = fleetSize - capitalCount;
    fleet.capitalCount = capitalCount;
    fleet.subcapCount  = subcapCount;

    // Assign roles based on aggression and support ratio.
    int supportSlots = static_cast<int>(
        static_cast<float>(subcapCount) * supportRatio);
    int dpsSlots     = subcapCount - supportSlots;

    uint32_t nextShipId = 0;

    // Capital ships.
    HullClass capitalHulls[] = {
        HullClass::Carrier, HullClass::Dreadnought, HullClass::Titan
    };
    for (int i = 0; i < capitalCount; ++i) {
        GeneratedFleetShip ship{};
        ship.shipId    = nextShipId++;
        ship.hullClass = capitalHulls[rng.range(0, 2)];
        ship.role      = (aggression > 0.6f) ? "dps" : "command";
        ship.shipName  = fleet.doctrineName + " Cap-" + std::to_string(i + 1);
        fleet.ships.push_back(ship);
    }

    // DPS subcaps.
    HullClass dpsHulls[] = {
        HullClass::Frigate, HullClass::Destroyer, HullClass::Cruiser,
        HullClass::Battlecruiser, HullClass::Battleship,
    };
    for (int i = 0; i < dpsSlots; ++i) {
        GeneratedFleetShip ship{};
        ship.shipId    = nextShipId++;
        ship.hullClass = dpsHulls[rng.range(0, 4)];
        ship.role      = "dps";
        ship.shipName  = fleet.doctrineName + " " + std::to_string(i + 1);
        fleet.ships.push_back(ship);
    }

    // Support subcaps (logistics, ewar, tackle).
    for (int i = 0; i < supportSlots; ++i) {
        GeneratedFleetShip ship{};
        ship.shipId    = nextShipId++;
        // Alternate support roles.
        int roleIdx = (i % 3) + 1;  // 1=logistics, 2=ewar, 3=tackle
        ship.role = kFleetRoles[roleIdx];
        ship.hullClass = (roleIdx == 1) ? HullClass::Cruiser
                       : (roleIdx == 2) ? HullClass::Frigate
                       :                  HullClass::Frigate;
        ship.shipName  = fleet.doctrineName + " Sup-" + std::to_string(i + 1);
        fleet.ships.push_back(ship);
    }

    fleet.valid = (!fleet.ships.empty() &&
                   static_cast<int>(fleet.ships.size()) == fleetSize);

    // Apply designer placements (pin specific ship assignments).
    int placementsApplied = 0;
    for (const auto& pe : style.placements) {
        if (pe.slotIndex < fleet.ships.size()) {
            auto& ship = fleet.ships[pe.slotIndex];
            if (pe.contentType <= static_cast<uint32_t>(HullClass::Titan)) {
                ship.hullClass = static_cast<HullClass>(pe.contentType);
            }
            if (!pe.label.empty()) {
                ship.shipName = pe.label;
            }
            ++placementsApplied;
        }
    }

    result.fleetResult       = fleet;
    result.placementsApplied = placementsApplied;
    result.parametersApplied = paramsApplied;
    result.success           = fleet.valid;
    if (!result.success) {
        result.errorMessage = "Fleet composition failed validation";
    }
    return result;
}

// ── Serialisation ──────────────────────────────────────────────────

std::string GenerationStyleEngine::serialize(const GenerationStyle& style) {
    std::ostringstream os;
    os << "STYLE_V1\n";
    os << "name=" << style.name << "\n";
    os << "type=" << static_cast<int>(style.type) << "\n";
    os << "version=" << style.version << "\n";
    os << "seed=" << style.baseSeed << "\n";

    os << "placements=" << style.placements.size() << "\n";
    for (const auto& pe : style.placements) {
        os << "P " << pe.slotIndex
           << " " << pe.posX << " " << pe.posY << " " << pe.posZ
           << " " << pe.contentType
           << " " << (pe.locked ? 1 : 0)
           << " " << pe.label << "\n";
    }

    os << "parameters=" << style.parameters.size() << "\n";
    for (const auto& p : style.parameters) {
        os << "A " << p.name
           << " " << p.value
           << " " << p.minValue << " " << p.maxValue
           << " " << (p.enabled ? 1 : 0) << "\n";
    }

    os << "END\n";
    return os.str();
}

GenerationStyle GenerationStyleEngine::deserialize(const std::string& data) {
    GenerationStyle style{};
    style.valid = false;

    std::istringstream is(data);
    std::string line;

    // Header.
    if (!std::getline(is, line) || line != "STYLE_V1") return style;

    auto readKeyVal = [&](const std::string& key) -> std::string {
        if (!std::getline(is, line)) return "";
        auto pos = line.find('=');
        if (pos == std::string::npos) return "";
        std::string k = line.substr(0, pos);
        if (k != key) return "";
        return line.substr(pos + 1);
    };

    style.name    = readKeyVal("name");
    style.type    = static_cast<GenerationStyleType>(
                        std::stoi(readKeyVal("type")));
    style.version = static_cast<uint32_t>(
                        std::stoul(readKeyVal("version")));
    style.baseSeed = std::stoull(readKeyVal("seed"));

    int pCount = std::stoi(readKeyVal("placements"));
    for (int i = 0; i < pCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        PlacementEntry pe{};
        int locked = 0;
        ls >> tag >> pe.slotIndex
           >> pe.posX >> pe.posY >> pe.posZ
           >> pe.contentType >> locked;
        pe.locked = (locked != 0);
        // Rest of line is the label.
        std::getline(ls >> std::ws, pe.label);
        style.placements.push_back(pe);
    }

    int aCount = std::stoi(readKeyVal("parameters"));
    for (int i = 0; i < aCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        ParameterOverride p{};
        int enabled = 0;
        ls >> tag >> p.name >> p.value >> p.minValue >> p.maxValue >> enabled;
        p.enabled = (enabled != 0);
        style.parameters.push_back(p);
    }

    style.valid = true;
    return style;
}

} // namespace pcg
} // namespace atlas
