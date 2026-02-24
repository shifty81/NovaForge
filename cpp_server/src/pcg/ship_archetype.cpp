#include "pcg/ship_archetype.h"

#include <sstream>
#include <algorithm>
#include <cmath>

namespace atlas {
namespace pcg {

// ── Subsystem type names ───────────────────────────────────────────

const char* ShipArchetypeEngine::subsystemTypeName(SubsystemType type) {
    switch (type) {
        case SubsystemType::Offensive:   return "Offensive";
        case SubsystemType::Defensive:   return "Defensive";
        case SubsystemType::Propulsion:  return "Propulsion";
        case SubsystemType::Engineering: return "Engineering";
        case SubsystemType::Core:        return "Core";
    }
    return "Unknown";
}

// ── Default archetype creation ─────────────────────────────────────

/// Hull-class specific defaults for archetype generation.
struct ArchetypeTemplate {
    int   turretCount;
    int   launcherCount;
    int   roomCount;
    int   deckCount;
    int   doorCount;
    float hullLength;        ///< Approximate hull length (metres).
    float hullWidth;
    float hullHeight;
    float shapeVariation;
    float sizeVariation;
    float detailVariation;
};

static const ArchetypeTemplate ARCHETYPE_TEMPLATES[] = {
    // Frigate:  2 turrets, 1 launcher, 3 rooms, 1 deck
    {  2, 1,  3, 1, 2,   30.0f, 10.0f,  6.0f, 0.15f, 0.10f, 0.20f },
    // Destroyer: 4 turrets, 2 launchers, 4 rooms, 1 deck
    {  4, 2,  4, 1, 3,   50.0f, 15.0f,  8.0f, 0.15f, 0.10f, 0.20f },
    // Cruiser: 4 turrets, 3 launchers, 6 rooms, 2 decks
    {  4, 3,  6, 2, 5,  100.0f, 25.0f, 15.0f, 0.20f, 0.15f, 0.25f },
    // Battlecruiser: 6 turrets, 4 launchers, 10 rooms, 2 decks
    {  6, 4, 10, 2, 7,  200.0f, 40.0f, 25.0f, 0.20f, 0.15f, 0.25f },
    // Battleship: 8 turrets, 4 launchers, 14 rooms, 3 decks
    {  8, 4, 14, 3, 10, 400.0f, 60.0f, 40.0f, 0.25f, 0.20f, 0.30f },
    // Capital: 10 turrets, 6 launchers, 25 rooms, 5 decks
    { 10, 6, 25, 5, 15, 800.0f, 120.0f, 80.0f, 0.25f, 0.20f, 0.30f },
};
static constexpr int ARCHETYPE_TEMPLATE_COUNT = 6;

/// Map a hull class to an archetype template index (0-5).
static int hullToTemplateIndex(HullClass hull) {
    HullClass base = baseHullClass(hull);
    switch (base) {
        case HullClass::Frigate:       return 0;
        case HullClass::Destroyer:     return 1;
        case HullClass::Cruiser:       return 2;
        case HullClass::Battlecruiser: return 3;
        case HullClass::Battleship:    return 4;
        case HullClass::Capital:       return 5;
        default:                       return 0;
    }
}

ShipArchetype ShipArchetypeEngine::createDefault(HullClass hull) {
    ShipArchetype arch{};
    arch.name      = std::string("Default ") +
                     ShipGenerator::hullClassName(hull) + " Archetype";
    arch.hullClass = hull;
    arch.version   = 1;
    arch.valid     = false;

    int idx = hullToTemplateIndex(hull);
    if (idx < 0 || idx >= ARCHETYPE_TEMPLATE_COUNT) idx = 0;
    const auto& tmpl = ARCHETYPE_TEMPLATES[idx];

    // ── Hull shape profile ─────────────────────────────────────────
    arch.hullShape.name      = "Hull " + ShipGenerator::hullClassName(hull);
    arch.hullShape.mirrorX   = true;   // Ships are symmetric.
    arch.hullShape.mirrorY   = false;
    arch.hullShape.smoothing = 0.5f;

    // Nose control point.
    ShapeControlPoint nose{};
    nose.posX = tmpl.hullLength * 0.5f;
    nose.posY = 0.0f;
    nose.posZ = 0.0f;
    nose.scaleX = 0.5f; nose.scaleY = 0.6f; nose.scaleZ = 0.6f;
    nose.weight = 1.0f;
    arch.hullShape.controlPoints.push_back(nose);

    // Mid-section (widest point).
    ShapeControlPoint mid{};
    mid.posX = 0.0f;
    mid.posY = 0.0f;
    mid.posZ = 0.0f;
    mid.scaleX = 1.0f; mid.scaleY = 1.0f; mid.scaleZ = 1.0f;
    mid.weight = 1.0f;
    arch.hullShape.controlPoints.push_back(mid);

    // Engine section (rear).
    ShapeControlPoint aft{};
    aft.posX = -tmpl.hullLength * 0.5f;
    aft.posY = 0.0f;
    aft.posZ = 0.0f;
    aft.scaleX = 0.7f; aft.scaleY = 0.8f; aft.scaleZ = 0.9f;
    aft.weight = 1.0f;
    arch.hullShape.controlPoints.push_back(aft);

    // ── Interior rooms ─────────────────────────────────────────────
    arch.deckCount = tmpl.deckCount;
    float roomSpacing = tmpl.hullLength / static_cast<float>(tmpl.roomCount + 1);

    for (int i = 0; i < tmpl.roomCount; ++i) {
        InteriorRoom room{};
        room.roomId = i;
        room.posX   = -tmpl.hullLength * 0.5f +
                      roomSpacing * static_cast<float>(i + 1);
        room.posY   = 0.0f;
        room.posZ   = 0.0f;
        room.dimX   = roomSpacing * 0.8f;
        room.dimY   = tmpl.hullWidth * 0.4f;
        room.dimZ   = tmpl.hullHeight / static_cast<float>(tmpl.deckCount);
        room.deckLevel    = i % tmpl.deckCount;
        room.pressurized  = true;
        room.powerDraw    = 1.0f;
        room.heatOutput   = 0.5f;
        room.crewCapacity = 4;

        // Assign room types cyclically.
        static const InteriorRoomType roomTypes[] = {
            InteriorRoomType::Cockpit,
            InteriorRoomType::Engineering,
            InteriorRoomType::WeaponControl,
            InteriorRoomType::CrewQuarters,
            InteriorRoomType::CargoBay,
            InteriorRoomType::Reactor,
            InteriorRoomType::ShieldCore,
            InteriorRoomType::Medbay,
        };
        static constexpr int typeCount = 8;
        room.type = roomTypes[i % typeCount];

        arch.rooms.push_back(room);
    }

    // ── Doors ──────────────────────────────────────────────────────
    for (int i = 0; i < tmpl.doorCount && i < tmpl.roomCount - 1; ++i) {
        DoorPlacement door{};
        door.doorId     = static_cast<uint32_t>(i);
        door.fromRoomId = i;
        door.toRoomId   = i + 1;
        // Position between the two rooms.
        door.posX   = (arch.rooms[static_cast<size_t>(i)].posX +
                       arch.rooms[static_cast<size_t>(i + 1)].posX) * 0.5f;
        door.posY   = 0.0f;
        door.posZ   = 0.0f;
        door.width  = 1.2f;
        door.height = 2.2f;
        door.isAirlock = false;
        door.locked    = true;  // Designer placement locked by default.
        arch.doors.push_back(door);
    }

    // ── Hardpoints ─────────────────────────────────────────────────
    WeaponSize wpnSize;
    switch (baseHullClass(hull)) {
        case HullClass::Frigate:       wpnSize = WeaponSize::Small;  break;
        case HullClass::Destroyer:     wpnSize = WeaponSize::Small;  break;
        case HullClass::Cruiser:       wpnSize = WeaponSize::Medium; break;
        case HullClass::Battlecruiser: wpnSize = WeaponSize::Medium; break;
        case HullClass::Battleship:    wpnSize = WeaponSize::Large;  break;
        case HullClass::Capital:       wpnSize = WeaponSize::XLarge; break;
        default:                       wpnSize = WeaponSize::Small;  break;
    }

    int totalHardpoints = tmpl.turretCount + tmpl.launcherCount;
    float hpSpacing = tmpl.hullLength / static_cast<float>(totalHardpoints + 1);

    for (int i = 0; i < totalHardpoints; ++i) {
        HardpointDefinition hp{};
        hp.hardpointId = static_cast<uint32_t>(i);
        hp.posX     = -tmpl.hullLength * 0.5f +
                      hpSpacing * static_cast<float>(i + 1);
        hp.posY     = (i % 2 == 0) ? tmpl.hullWidth * 0.25f
                                    : -tmpl.hullWidth * 0.25f;
        hp.posZ     = (i < tmpl.turretCount) ? tmpl.hullHeight * 0.3f
                                             : -tmpl.hullHeight * 0.2f;
        hp.facingDeg = 0.0f;   // Forward.
        hp.arcDeg    = (i < tmpl.turretCount) ? 120.0f : 60.0f;
        hp.size      = wpnSize;
        hp.isDorsal  = (i < tmpl.turretCount);
        hp.groupTag  = (hp.posX > 0.0f) ? "fore" : "aft";
        arch.hardpoints.push_back(hp);
    }

    // ── Subsystem slots (T3 style) ─────────────────────────────────
    for (int i = 0; i < SUBSYSTEM_TYPE_COUNT; ++i) {
        SubsystemSlot slot{};
        slot.type = static_cast<SubsystemType>(i);
        slot.activeVariant = -1;

        // Place each subsystem at a different region of the hull.
        switch (slot.type) {
            case SubsystemType::Offensive:
                slot.regionPosX = tmpl.hullLength * 0.3f;
                slot.regionPosY = 0.0f;
                slot.regionPosZ = tmpl.hullHeight * 0.2f;
                break;
            case SubsystemType::Defensive:
                slot.regionPosX = 0.0f;
                slot.regionPosY = 0.0f;
                slot.regionPosZ = 0.0f;
                break;
            case SubsystemType::Propulsion:
                slot.regionPosX = -tmpl.hullLength * 0.4f;
                slot.regionPosY = 0.0f;
                slot.regionPosZ = 0.0f;
                break;
            case SubsystemType::Engineering:
                slot.regionPosX = -tmpl.hullLength * 0.2f;
                slot.regionPosY = 0.0f;
                slot.regionPosZ = -tmpl.hullHeight * 0.1f;
                break;
            case SubsystemType::Core:
                slot.regionPosX = tmpl.hullLength * 0.1f;
                slot.regionPosY = 0.0f;
                slot.regionPosZ = tmpl.hullHeight * 0.1f;
                break;
        }
        slot.regionRadius = tmpl.hullLength * 0.15f;

        // Create 3 variants per subsystem type.
        for (int v = 0; v < 3; ++v) {
            SubsystemVariant var{};
            var.type = slot.type;

            switch (slot.type) {
                case SubsystemType::Offensive:
                    if (v == 0) {
                        var.name = "Assault Batteries";
                        var.dpsBonus = 0.15f;
                    } else if (v == 1) {
                        var.name = "Sniper Configuration";
                        var.dpsBonus = 0.10f;
                    } else {
                        var.name = "Drone Bay Extension";
                        var.dpsBonus = 0.05f;
                    }
                    break;
                case SubsystemType::Defensive:
                    if (v == 0) {
                        var.name = "Shield Amplifier";
                        var.shieldBonus = 0.20f;
                    } else if (v == 1) {
                        var.name = "Armor Reinforcement";
                        var.hullBonus = 0.20f;
                    } else {
                        var.name = "Adaptive Plating";
                        var.shieldBonus = 0.10f;
                        var.hullBonus   = 0.10f;
                    }
                    break;
                case SubsystemType::Propulsion:
                    if (v == 0) {
                        var.name = "Afterburner Nacelles";
                        var.speedBonus = 0.20f;
                    } else if (v == 1) {
                        var.name = "Warp Stabilizer";
                        var.speedBonus = 0.10f;
                    } else {
                        var.name = "Interdiction Nullifier";
                        var.speedBonus = 0.05f;
                    }
                    break;
                case SubsystemType::Engineering:
                    if (v == 0) {
                        var.name = "Capacitor Flux Coil";
                        var.hullBonus = 0.05f;
                    } else if (v == 1) {
                        var.name = "Power Diagnostic";
                        var.hullBonus   = 0.05f;
                        var.shieldBonus = 0.05f;
                    } else {
                        var.name = "Emergency Repair";
                        var.hullBonus = 0.15f;
                    }
                    break;
                case SubsystemType::Core:
                    if (v == 0) {
                        var.name = "Electronic Warfare";
                        var.dpsBonus = 0.05f;
                    } else if (v == 1) {
                        var.name = "Covert Reconfiguration";
                        var.speedBonus = 0.05f;
                    } else {
                        var.name = "Augmented Sensor";
                        var.shieldBonus = 0.05f;
                    }
                    break;
            }

            // Give each variant a shape modifier.
            var.shapeModifier.name = var.name + " shape";
            var.shapeModifier.smoothing = 0.6f;
            ShapeControlPoint cp{};
            cp.posX = slot.regionPosX;
            cp.posY = slot.regionPosY;
            cp.posZ = slot.regionPosZ;
            // Different variants deform the hull differently.
            cp.scaleX = 1.0f + 0.1f * static_cast<float>(v);
            cp.scaleY = 1.0f + 0.05f * static_cast<float>(v);
            cp.scaleZ = 1.0f + 0.08f * static_cast<float>(v);
            cp.weight = 0.8f;
            var.shapeModifier.controlPoints.push_back(cp);

            slot.variants.push_back(var);
        }

        arch.subsystems.push_back(slot);
    }

    // ── Module visual rules ────────────────────────────────────────
    arch.moduleVisuals.push_back(
        {"shield", "emitter",  0.5f, 1.5f, 0.0f, 0.0f, tmpl.hullHeight * 0.2f});
    arch.moduleVisuals.push_back(
        {"armor",  "plating",  0.3f, 1.2f, 0.0f, 0.0f, 0.0f});
    arch.moduleVisuals.push_back(
        {"weapon", "barrel",   0.8f, 2.0f, tmpl.hullLength * 0.3f, 0.0f, 0.0f});
    arch.moduleVisuals.push_back(
        {"engine", "nacelle",  0.6f, 1.8f, -tmpl.hullLength * 0.4f, 0.0f, 0.0f});

    // ── Variation bounds ───────────────────────────────────────────
    arch.shapeVariation  = tmpl.shapeVariation;
    arch.sizeVariation   = tmpl.sizeVariation;
    arch.detailVariation = tmpl.detailVariation;

    return arch;
}

// ── Validation ─────────────────────────────────────────────────────

bool ShipArchetypeEngine::validate(ShipArchetype& archetype) {
    if (archetype.name.empty()) {
        archetype.valid = false;
        return false;
    }

    if (archetype.hullShape.controlPoints.empty()) {
        archetype.valid = false;
        return false;
    }

    if (archetype.rooms.empty()) {
        archetype.valid = false;
        return false;
    }

    if (archetype.hardpoints.empty()) {
        archetype.valid = false;
        return false;
    }

    // Hardpoint IDs must be unique.
    std::vector<uint32_t> hpIds;
    for (const auto& hp : archetype.hardpoints) {
        if (std::find(hpIds.begin(), hpIds.end(), hp.hardpointId)
                != hpIds.end()) {
            archetype.valid = false;
            return false;
        }
        hpIds.push_back(hp.hardpointId);
    }

    // Door room references must exist.
    for (const auto& door : archetype.doors) {
        bool foundFrom = false, foundTo = false;
        for (const auto& room : archetype.rooms) {
            if (room.roomId == door.fromRoomId) foundFrom = true;
            if (room.roomId == door.toRoomId)   foundTo   = true;
        }
        if (!foundFrom || !foundTo) {
            archetype.valid = false;
            return false;
        }
    }

    archetype.valid = true;
    return true;
}

// ── PCG variation helpers ──────────────────────────────────────────

void ShipArchetypeEngine::varyHardpoints(
        DeterministicRNG& rng,
        std::vector<HardpointDefinition>& hardpoints,
        float variation) {
    for (auto& hp : hardpoints) {
        // Add small positional jitter within variation bounds.
        hp.posX += rng.rangeFloat(-variation, variation) * 2.0f;
        hp.posY += rng.rangeFloat(-variation, variation) * 1.0f;
        hp.posZ += rng.rangeFloat(-variation, variation) * 0.5f;

        // Small arc variation.
        hp.arcDeg += rng.rangeFloat(-5.0f, 5.0f) * variation;
        if (hp.arcDeg < 15.0f)  hp.arcDeg = 15.0f;
        if (hp.arcDeg > 360.0f) hp.arcDeg = 360.0f;
    }
}

void ShipArchetypeEngine::varyDoors(
        DeterministicRNG& rng,
        std::vector<DoorPlacement>& doors,
        float variation) {
    for (auto& door : doors) {
        if (door.locked) continue;  // Respect designer locks.
        door.posX += rng.rangeFloat(-variation, variation) * 0.5f;
        door.posY += rng.rangeFloat(-variation, variation) * 0.3f;
    }
}

void ShipArchetypeEngine::varyShipStats(
        DeterministicRNG& rng,
        GeneratedShip& ship,
        float sizeVariation) {
    // Scale ship stats within variation bounds.
    float scale = 1.0f + rng.rangeFloat(-sizeVariation, sizeVariation);
    ship.mass            *= scale;
    ship.armorHP         *= scale;
    ship.shieldHP        *= (1.0f + rng.rangeFloat(-sizeVariation * 0.5f,
                                                     sizeVariation * 0.5f));
    ship.signatureRadius *= (1.0f + rng.rangeFloat(-sizeVariation * 0.3f,
                                                     sizeVariation * 0.3f));
}

// ── Generate from archetype ────────────────────────────────────────

ArchetypeVariant ShipArchetypeEngine::generateFromArchetype(
        const PCGContext& ctx, const ShipArchetype& archetype) {
    ArchetypeVariant result{};
    result.archetypeName = archetype.name;
    result.valid         = false;

    DeterministicRNG rng(ctx.seed);

    // 1) Generate the base ship using the standard generator.
    GeneratedShip ship = ShipGenerator::generate(ctx, archetype.hullClass);

    // 2) Override turret/launcher slots from archetype hardpoints.
    int turretCount   = 0;
    int launcherCount = 0;
    for (const auto& hp : archetype.hardpoints) {
        if (hp.isDorsal) ++turretCount;
        else             ++launcherCount;
    }
    ship.turretSlots   = turretCount;
    ship.launcherSlots = launcherCount;

    // 3) Apply PCG stat variation.
    varyShipStats(rng, ship, archetype.sizeVariation);

    // 4) Copy and vary hardpoints.
    result.hardpoints = archetype.hardpoints;
    varyHardpoints(rng, result.hardpoints, archetype.shapeVariation);

    // 5) Copy and vary doors.
    result.doors = archetype.doors;
    varyDoors(rng, result.doors, archetype.shapeVariation);

    // 6) Generate interior from archetype's ship class.
    int shipClassIdx = hullToTemplateIndex(archetype.hullClass);
    result.interior = InteriorGenerator::generate(ctx, shipClassIdx);

    // 7) Apply archetype room types over the generated interior.
    for (size_t i = 0; i < archetype.rooms.size() &&
                        i < result.interior.rooms.size(); ++i) {
        result.interior.rooms[i].type = archetype.rooms[i].type;
        // Respect designer positions (blend with generated).
        float blend = 1.0f - archetype.shapeVariation;
        result.interior.rooms[i].posX =
            archetype.rooms[i].posX * blend +
            result.interior.rooms[i].posX * (1.0f - blend);
        result.interior.rooms[i].posY =
            archetype.rooms[i].posY * blend +
            result.interior.rooms[i].posY * (1.0f - blend);
    }

    // 8) Initialize subsystems to none active.
    result.activeSubsystems.resize(archetype.subsystems.size(), -1);

    // Count variations applied.
    result.variationsApplied = static_cast<int>(
        archetype.hardpoints.size() + archetype.doors.size());

    result.ship  = ship;
    result.valid = ship.valid;
    return result;
}

// ── Apply subsystems (T3-style fitting change) ─────────────────────

void ShipArchetypeEngine::applySubsystems(
        ArchetypeVariant& variant,
        const ShipArchetype& archetype,
        const std::vector<int>& activeVariants) {
    variant.activeSubsystems = activeVariants;

    for (size_t i = 0; i < archetype.subsystems.size() &&
                        i < activeVariants.size(); ++i) {
        int vi = activeVariants[i];
        if (vi < 0 || vi >= static_cast<int>(
                archetype.subsystems[i].variants.size())) {
            continue;
        }

        const auto& var = archetype.subsystems[i].variants[
            static_cast<size_t>(vi)];

        // Apply stat bonuses.
        variant.ship.armorHP  *= (1.0f + var.hullBonus);
        variant.ship.shieldHP *= (1.0f + var.shieldBonus);
        variant.ship.thrust   *= (1.0f + var.speedBonus);

        // Apply shape modifier to the ship geometry.
        AssetStyleLibrary::applyShapeToShip(
            variant.ship, var.shapeModifier);
    }
}

// ── Apply module visuals ───────────────────────────────────────────

void ShipArchetypeEngine::applyModuleVisuals(
        ArchetypeVariant& variant,
        const ShipArchetype& archetype,
        const std::vector<std::string>& fittedModules) {
    for (const auto& moduleCat : fittedModules) {
        for (const auto& rule : archetype.moduleVisuals) {
            if (rule.moduleCategory == moduleCat) {
                // Module visual rules affect the ship's geometry.
                // For now, we apply a mass/sig scaling to represent
                // the visual change.  In a full renderer this would
                // spawn additional geometry at the rule's position.
                float scale = (rule.scaleMin + rule.scaleMax) * 0.5f;
                variant.ship.mass *= (1.0f + 0.01f * scale);
                variant.ship.signatureRadius *= (1.0f + 0.005f * scale);
            }
        }
    }
}

// ── Serialisation ──────────────────────────────────────────────────

std::string ShipArchetypeEngine::serialize(const ShipArchetype& arch) {
    std::ostringstream os;
    os << "ARCHETYPE_V1\n";
    os << "name=" << arch.name << "\n";
    os << "hullClass=" << static_cast<int>(arch.hullClass) << "\n";
    os << "version=" << arch.version << "\n";
    os << "deckCount=" << arch.deckCount << "\n";
    os << "shapeVariation=" << arch.shapeVariation << "\n";
    os << "sizeVariation=" << arch.sizeVariation << "\n";
    os << "detailVariation=" << arch.detailVariation << "\n";

    // Hull shape.
    os << "hullShape_name=" << arch.hullShape.name << "\n";
    os << "hullShape_mirrorX=" << (arch.hullShape.mirrorX ? 1 : 0) << "\n";
    os << "hullShape_smoothing=" << arch.hullShape.smoothing << "\n";
    os << "hullShape_points=" << arch.hullShape.controlPoints.size() << "\n";
    for (const auto& cp : arch.hullShape.controlPoints) {
        os << "CP " << cp.posX << " " << cp.posY << " " << cp.posZ
           << " " << cp.scaleX << " " << cp.scaleY << " " << cp.scaleZ
           << " " << cp.weight << "\n";
    }

    // Rooms.
    os << "rooms=" << arch.rooms.size() << "\n";
    for (const auto& r : arch.rooms) {
        os << "R " << r.roomId << " " << static_cast<int>(r.type)
           << " " << r.posX << " " << r.posY << " " << r.posZ
           << " " << r.dimX << " " << r.dimY << " " << r.dimZ
           << " " << r.deckLevel << "\n";
    }

    // Doors.
    os << "doors=" << arch.doors.size() << "\n";
    for (const auto& d : arch.doors) {
        os << "D " << d.doorId << " " << d.fromRoomId << " " << d.toRoomId
           << " " << d.posX << " " << d.posY << " " << d.posZ
           << " " << d.width << " " << d.height
           << " " << (d.isAirlock ? 1 : 0)
           << " " << (d.locked ? 1 : 0) << "\n";
    }

    // Hardpoints.
    os << "hardpoints=" << arch.hardpoints.size() << "\n";
    for (const auto& hp : arch.hardpoints) {
        os << "H " << hp.hardpointId
           << " " << hp.posX << " " << hp.posY << " " << hp.posZ
           << " " << hp.facingDeg << " " << hp.arcDeg
           << " " << static_cast<int>(hp.size)
           << " " << (hp.isDorsal ? 1 : 0)
           << " " << hp.groupTag << "\n";
    }

    // Subsystems.
    os << "subsystems=" << arch.subsystems.size() << "\n";
    for (const auto& ss : arch.subsystems) {
        os << "SS " << static_cast<int>(ss.type)
           << " " << ss.regionPosX << " " << ss.regionPosY
           << " " << ss.regionPosZ << " " << ss.regionRadius
           << " " << ss.activeVariant
           << " " << ss.variants.size() << "\n";
        for (const auto& v : ss.variants) {
            os << "SV " << v.name
               << "|" << v.hullBonus << "|" << v.shieldBonus
               << "|" << v.speedBonus << "|" << v.dpsBonus << "\n";
        }
    }

    // Module visual rules.
    os << "moduleVisuals=" << arch.moduleVisuals.size() << "\n";
    for (const auto& mv : arch.moduleVisuals) {
        os << "MV " << mv.moduleCategory << " " << mv.effectType
           << " " << mv.scaleMin << " " << mv.scaleMax
           << " " << mv.posX << " " << mv.posY << " " << mv.posZ << "\n";
    }

    os << "END\n";
    return os.str();
}

ShipArchetype ShipArchetypeEngine::deserialize(const std::string& data) {
    ShipArchetype arch{};
    arch.valid = false;

    std::istringstream is(data);
    std::string line;

    if (!std::getline(is, line) || line != "ARCHETYPE_V1") return arch;

    auto readVal = [&](const std::string& key) -> std::string {
        if (!std::getline(is, line)) return "";
        auto pos = line.find('=');
        if (pos == std::string::npos) return "";
        if (line.substr(0, pos) != key) return "";
        return line.substr(pos + 1);
    };

    arch.name            = readVal("name");
    arch.hullClass       = static_cast<HullClass>(
                               std::stoi(readVal("hullClass")));
    arch.version         = static_cast<uint32_t>(
                               std::stoul(readVal("version")));
    arch.deckCount       = std::stoi(readVal("deckCount"));
    arch.shapeVariation  = std::stof(readVal("shapeVariation"));
    arch.sizeVariation   = std::stof(readVal("sizeVariation"));
    arch.detailVariation = std::stof(readVal("detailVariation"));

    // Hull shape.
    arch.hullShape.name      = readVal("hullShape_name");
    arch.hullShape.mirrorX   = (readVal("hullShape_mirrorX") == "1");
    arch.hullShape.smoothing = std::stof(readVal("hullShape_smoothing"));

    int cpCount = std::stoi(readVal("hullShape_points"));
    for (int i = 0; i < cpCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        ShapeControlPoint cp{};
        ls >> tag >> cp.posX >> cp.posY >> cp.posZ
           >> cp.scaleX >> cp.scaleY >> cp.scaleZ >> cp.weight;
        arch.hullShape.controlPoints.push_back(cp);
    }

    // Rooms.
    int roomCount = std::stoi(readVal("rooms"));
    for (int i = 0; i < roomCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        InteriorRoom r{};
        int typeVal = 0;
        ls >> tag >> r.roomId >> typeVal >> r.posX >> r.posY >> r.posZ
           >> r.dimX >> r.dimY >> r.dimZ >> r.deckLevel;
        r.type = static_cast<InteriorRoomType>(typeVal);
        r.pressurized = true;
        arch.rooms.push_back(r);
    }

    // Doors.
    int doorCount = std::stoi(readVal("doors"));
    for (int i = 0; i < doorCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        DoorPlacement d{};
        int airlock = 0, locked = 0;
        ls >> tag >> d.doorId >> d.fromRoomId >> d.toRoomId
           >> d.posX >> d.posY >> d.posZ >> d.width >> d.height
           >> airlock >> locked;
        d.isAirlock = (airlock != 0);
        d.locked    = (locked != 0);
        arch.doors.push_back(d);
    }

    // Hardpoints.
    int hpCount = std::stoi(readVal("hardpoints"));
    for (int i = 0; i < hpCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        HardpointDefinition hp{};
        int sizeVal = 0, dorsal = 0;
        ls >> tag >> hp.hardpointId
           >> hp.posX >> hp.posY >> hp.posZ
           >> hp.facingDeg >> hp.arcDeg >> sizeVal >> dorsal;
        hp.size     = static_cast<WeaponSize>(sizeVal);
        hp.isDorsal = (dorsal != 0);
        std::getline(ls >> std::ws, hp.groupTag);
        arch.hardpoints.push_back(hp);
    }

    // Subsystems.
    int ssCount = std::stoi(readVal("subsystems"));
    for (int i = 0; i < ssCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        SubsystemSlot ss{};
        int typeVal = 0, varCount = 0;
        ls >> tag >> typeVal >> ss.regionPosX >> ss.regionPosY
           >> ss.regionPosZ >> ss.regionRadius >> ss.activeVariant
           >> varCount;
        ss.type = static_cast<SubsystemType>(typeVal);

        for (int v = 0; v < varCount; ++v) {
            if (!std::getline(is, line)) break;
            // Format: "SV name|hull|shield|speed|dps"
            auto barPos = line.find('|');
            SubsystemVariant sv{};
            if (barPos != std::string::npos) {
                sv.name = line.substr(3, barPos - 3);  // Skip "SV "
                std::istringstream vs(line.substr(barPos + 1));
                char sep;
                vs >> sv.hullBonus >> sep >> sv.shieldBonus
                   >> sep >> sv.speedBonus >> sep >> sv.dpsBonus;
            }
            sv.type = ss.type;
            ss.variants.push_back(sv);
        }

        arch.subsystems.push_back(ss);
    }

    // Module visual rules.
    int mvCount = std::stoi(readVal("moduleVisuals"));
    for (int i = 0; i < mvCount; ++i) {
        if (!std::getline(is, line)) break;
        std::istringstream ls(line);
        std::string tag;
        ModuleVisualRule mv{};
        ls >> tag >> mv.moduleCategory >> mv.effectType
           >> mv.scaleMin >> mv.scaleMax
           >> mv.posX >> mv.posY >> mv.posZ;
        arch.moduleVisuals.push_back(mv);
    }

    arch.valid = true;
    return arch;
}

} // namespace pcg
} // namespace atlas
