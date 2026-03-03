#include "PCGPreviewPanel.h"
#include <sstream>

namespace atlas::editor {

// ── Helpers ─────────────────────────────────────────────────────────

static const char* previewModeName(PCGPreviewMode mode) {
    switch (mode) {
        case PCGPreviewMode::Ship:            return "Ship";
        case PCGPreviewMode::Station:         return "Station";
        case PCGPreviewMode::Interior:        return "Interior";
        case PCGPreviewMode::Character:       return "Character";
        case PCGPreviewMode::SpineHull:       return "SpineHull";
        case PCGPreviewMode::TurretPlacement: return "TurretPlacement";
    }
    return "Unknown";
}

static const char* stationModuleTypeName(pcg::StationModuleType type) {
    switch (type) {
        case pcg::StationModuleType::Habitat:    return "Habitat";
        case pcg::StationModuleType::Lab:        return "Lab";
        case pcg::StationModuleType::DockingBay: return "DockingBay";
        case pcg::StationModuleType::Hangar:     return "Hangar";
        case pcg::StationModuleType::Corridor:   return "Corridor";
        case pcg::StationModuleType::Power:      return "Power";
        case pcg::StationModuleType::Storage:    return "Storage";
    }
    return "Unknown";
}

// ── Construction ────────────────────────────────────────────────────

PCGPreviewPanel::PCGPreviewPanel() {
    m_pcgManager.initialize(m_settings.seed);
}

// ── Draw (stub – real UI rendered via Atlas UI in editor) ───────────

void PCGPreviewPanel::Draw() {
    // In headless / test mode this is a no-op.
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "PCG Preview", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Mode selector
    static const std::vector<std::string> modeItems = {
        "Ship", "Station", "Interior", "Character", "SpineHull", "TurretPlacement"
    };
    int modeIdx = static_cast<int>(m_settings.mode);
    atlas::Rect comboRect{b.x + pad, y, b.w - 2.0f * pad, rowH + pad};
    if (atlas::comboBox(ctx, "Mode", comboRect, modeItems, &modeIdx, &m_modeDropdownOpen)) {
        m_settings.mode = static_cast<PCGPreviewMode>(modeIdx);
    }
    y += rowH + pad + pad;

    // Seed label
    atlas::label(ctx, {b.x + pad, y},
        "Seed: " + std::to_string(m_settings.seed), ctx.theme().textPrimary);
    y += rowH + pad;

    // Randomize button
    const float btnW = 100.0f;
    if (atlas::button(ctx, "Randomize", {b.x + pad, y, btnW, rowH + pad})) {
        Randomize();
    }
    y += rowH + pad + pad;

    // Generate button
    if (atlas::button(ctx, "Generate", {b.x + pad, y, btnW, rowH + pad})) {
        Generate();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Generation ──────────────────────────────────────────────────────

void PCGPreviewPanel::Generate() {
    m_log.clear();

    // Re-initialize the manager with the current seed so that every
    // Generate() call with the same seed is deterministic.
    m_pcgManager.initialize(m_settings.seed);

    log("Generating " + std::string(previewModeName(m_settings.mode))
        + " with seed " + std::to_string(m_settings.seed)
        + " (v" + std::to_string(m_settings.version) + ")");

    switch (m_settings.mode) {
        case PCGPreviewMode::Ship:            generateShip();            break;
        case PCGPreviewMode::Station:         generateStation();         break;
        case PCGPreviewMode::Interior:        generateInterior();        break;
        case PCGPreviewMode::Character:       generateCharacter();       break;
        case PCGPreviewMode::SpineHull:       generateSpineHull();       break;
        case PCGPreviewMode::TurretPlacement: generateTurretPlacement(); break;
    }
}

void PCGPreviewPanel::Randomize() {
    // Simple deterministic bump so that consecutive calls yield
    // different seeds yet remain reproducible from the same starting
    // point across machines.
    m_settings.seed = pcg::hashCombine(m_settings.seed, m_settings.seed + 1);
    Generate();
}

void PCGPreviewPanel::ClearPreview() {
    m_shipPreview             = ShipPreview{};
    m_stationPreview          = StationPreview{};
    m_interiorPreview         = InteriorPreview{};
    m_characterPreview        = CharacterPreview{};
    m_spineHullPreview        = SpineHullPreview{};
    m_turretPlacementPreview  = TurretPlacementPreview{};
    m_log.clear();
}

// ── Ship generation ─────────────────────────────────────────────────

void PCGPreviewPanel::generateShip() {
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/1, m_settings.version);

    pcg::GeneratedShip ship = m_settings.overrideHull
        ? pcg::ShipGenerator::generate(ctx, m_settings.hullClass)
        : pcg::ShipGenerator::generate(ctx);

    m_shipPreview.data      = ship;
    m_shipPreview.populated = true;

    // Summarize into the log.
    std::ostringstream os;
    os << "Ship: " << ship.shipName
       << " | Hull: " << pcg::ShipGenerator::hullClassName(ship.hullClass)
       << " | Mass: " << ship.mass << " t"
       << " | Thrust: " << ship.thrust << " N"
       << " | Turrets: " << ship.turretSlots
       << " | Launchers: " << ship.launcherSlots
       << " | Armor: " << ship.armorHP
       << " | Shield: " << ship.shieldHP
       << " | Valid: " << (ship.valid ? "yes" : "NO");
    log(os.str());
}

// ── Station generation ──────────────────────────────────────────────

void PCGPreviewPanel::generateStation() {
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Station, /*objectId=*/1, m_settings.version);

    pcg::GeneratedStation station = m_settings.overrideModuleCount
        ? pcg::StationGenerator::generate(ctx, m_settings.moduleCount)
        : pcg::StationGenerator::generate(ctx);

    m_stationPreview.data      = station;
    m_stationPreview.populated = true;

    std::ostringstream os;
    os << "Station: " << station.modules.size() << " modules"
       << " | Power: " << station.totalPowerProduction
       << " / " << station.totalPowerConsumption
       << " | Valid: " << (station.valid ? "yes" : "NO");
    log(os.str());

    for (const auto& mod : station.modules) {
        std::ostringstream ms;
        ms << "  Module " << mod.moduleId
           << ": " << stationModuleTypeName(mod.type)
           << " (" << mod.dimX << "x" << mod.dimY << "x" << mod.dimZ << " m)";
        log(ms.str());
    }
}

// ── Interior generation ─────────────────────────────────────────────

void PCGPreviewPanel::generateInterior() {
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/1, m_settings.version);

    pcg::GeneratedInterior interior =
        pcg::InteriorGenerator::generate(ctx, m_settings.shipClass);

    m_interiorPreview.data      = interior;
    m_interiorPreview.populated = true;

    std::ostringstream os;
    os << "Interior: " << interior.rooms.size() << " rooms, "
       << interior.corridors.size() << " corridors, "
       << interior.deckCount << " decks"
       << " | Valid: " << (interior.valid ? "yes" : "NO");
    log(os.str());

    for (const auto& room : interior.rooms) {
        std::ostringstream rs;
        rs << "  Room " << room.roomId
           << ": " << pcg::interiorRoomTypeName(room.type)
           << " (" << room.dimX << "x" << room.dimY << "x" << room.dimZ << " m)"
           << " deck " << room.deckLevel;
        log(rs.str());
    }
}

// ── Character generation ─────────────────────────────────────────────

void PCGPreviewPanel::generateCharacter() {
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Character, /*objectId=*/1, m_settings.version);

    pcg::GeneratedLowPolyCharacter character;
    if (m_settings.overrideArchetype && m_settings.overrideGender) {
        character = pcg::LowPolyCharacterGenerator::generate(
            ctx, m_settings.characterArchetype, m_settings.characterIsMale);
    } else if (m_settings.overrideArchetype) {
        character = pcg::LowPolyCharacterGenerator::generate(
            ctx, m_settings.characterArchetype);
    } else {
        character = pcg::LowPolyCharacterGenerator::generate(ctx);
    }

    m_characterPreview.data      = character;
    m_characterPreview.populated = true;

    std::ostringstream os;
    os << "Character: "
       << pcg::LowPolyCharacterGenerator::archetypeClassName(character.archetype)
       << " | Gender: " << (character.isMale ? "Male" : "Female")
       << " | Body parts: " << character.bodyParts.size()
       << " | Clothing: " << character.clothing.size()
       << " | Palette regions: " << character.palette.size()
       << " | Skeleton: " << character.skeletonId
       << " | FlatShaded: " << (character.flatShaded ? "yes" : "no")
       << " | VertexColors: " << (character.useVertexColors ? "yes" : "no")
       << " | Valid: " << (character.valid ? "yes" : "NO");
    log(os.str());

    for (const auto& part : character.bodyParts) {
        std::ostringstream ps;
        ps << "  Body: " << part.variant << " (" << part.meshFile << ")";
        log(ps.str());
    }
    for (const auto& item : character.clothing) {
        std::ostringstream cs;
        cs << "  Clothing: " << item.variant << " (" << item.meshFile << ")";
        log(cs.str());
    }
    for (const auto& region : character.palette) {
        if (region.chosen >= 0 &&
            region.chosen < static_cast<int>(region.colors.size())) {
            auto& c = region.colors[static_cast<size_t>(region.chosen)];
            std::ostringstream rs;
            rs << "  Palette [" << region.regionName << "]: RGB("
               << c.r << ", " << c.g << ", " << c.b << ")";
            log(rs.str());
        }
    }

    // Log FPS arm config
    log("  FPS Arms: L=" + character.fpsArms.leftArmMesh
        + " R=" + character.fpsArms.rightArmMesh);
}

// ── Spine Hull generation ────────────────────────────────────────────

void PCGPreviewPanel::generateSpineHull() {
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/1, m_settings.version);

    pcg::GeneratedSpineHull hull;
    if (m_settings.overrideHull && m_settings.overrideFaction) {
        hull = pcg::SpineHullGenerator::generate(ctx, m_settings.hullClass, m_settings.faction);
    } else if (m_settings.overrideHull) {
        hull = pcg::SpineHullGenerator::generate(ctx, m_settings.hullClass);
    } else {
        hull = pcg::SpineHullGenerator::generate(ctx);
    }

    m_spineHullPreview.data      = hull;
    m_spineHullPreview.populated = true;

    std::ostringstream os;
    os << "SpineHull: " << pcg::SpineHullGenerator::spineTypeName(hull.spine)
       << " | Class: " << pcg::ShipGenerator::hullClassName(hull.hull_class)
       << " | Length: " << hull.profile.length << " m"
       << " | Width(fwd/mid/aft): " << hull.profile.width_fwd
       << "/" << hull.profile.width_mid
       << "/" << hull.profile.width_aft << " m"
       << " | Aspect: " << hull.aspect_ratio
       << " | Engines: " << hull.engine_cluster_count
       << " | Greeble: " << hull.total_greeble_count
       << " | Symmetric: " << (hull.bilateral_symmetry ? "yes" : "no")
       << " | Faction: " << (hull.faction_style.empty() ? "none" : hull.faction_style)
       << " | Valid: " << (hull.valid ? "yes" : "NO");
    log(os.str());

    for (size_t i = 0; i < hull.zones.size(); ++i) {
        const auto& z = hull.zones[i];
        const char* zoneName = "Unknown";
        switch (z.zone) {
            case pcg::FunctionalZone::Command:     zoneName = "Command";     break;
            case pcg::FunctionalZone::MidHull:     zoneName = "MidHull";     break;
            case pcg::FunctionalZone::Engineering: zoneName = "Engineering"; break;
        }
        std::ostringstream zs;
        zs << "  Zone " << i << ": " << zoneName
           << " (" << static_cast<int>(z.length_fraction * 100.0f) << "% spine)"
           << " greeble=" << z.greeble_count;
        log(zs.str());
    }
}

// ── Turret Placement generation ──────────────────────────────────────

void PCGPreviewPanel::generateTurretPlacement() {
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/1, m_settings.version);

    // First generate a spine hull to provide hull class context.
    pcg::GeneratedSpineHull hull;
    if (m_settings.overrideHull && m_settings.overrideFaction) {
        hull = pcg::SpineHullGenerator::generate(ctx, m_settings.hullClass, m_settings.faction);
    } else if (m_settings.overrideHull) {
        hull = pcg::SpineHullGenerator::generate(ctx, m_settings.hullClass);
    } else {
        hull = pcg::SpineHullGenerator::generate(ctx);
    }

    // Derive turret slot count from the generated ship if not overridden.
    int slots = m_settings.turretSlots;
    if (!m_settings.overrideTurretSlots) {
        pcg::GeneratedShip ship = pcg::ShipGenerator::generate(ctx, hull.hull_class);
        slots = ship.turretSlots;
    }

    // Use a child context so turret placement RNG is independent.
    pcg::PCGContext turretCtx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, /*objectId=*/2, m_settings.version);

    pcg::TurretPlacement placement = pcg::TurretPlacementSystem::place(
        turretCtx, hull.hull_class, slots,
        m_settings.overrideFaction ? m_settings.faction : hull.faction_style);

    m_turretPlacementPreview.data      = placement;
    m_turretPlacementPreview.hull      = hull;
    m_turretPlacementPreview.populated = true;

    std::ostringstream os;
    os << "TurretPlacement on "
       << pcg::SpineHullGenerator::spineTypeName(hull.spine)
       << " " << pcg::ShipGenerator::hullClassName(hull.hull_class)
       << " | Mounts: " << placement.mounts.size()
       << " | Coverage: " << static_cast<int>(placement.coverage_score * 100.0f) << "%"
       << " | MaxOverlap: " << static_cast<int>(placement.max_overlap * 100.0f) << "%"
       << " | Valid: " << (placement.valid ? "yes" : "NO");
    log(os.str());

    for (size_t i = 0; i < placement.mounts.size(); ++i) {
        const auto& m = placement.mounts[i];
        const char* sizeName = "Unknown";
        switch (m.size) {
            case pcg::TurretSize::Small:   sizeName = "Small";   break;
            case pcg::TurretSize::Medium:  sizeName = "Medium";  break;
            case pcg::TurretSize::Large:   sizeName = "Large";   break;
            case pcg::TurretSize::Capital: sizeName = "Capital"; break;
        }
        std::ostringstream ms;
        ms << "  Mount " << m.socket_id << ": " << sizeName
           << " pos(" << m.x_offset << ", " << m.y_offset << ", " << m.z_offset << ")"
           << " facing=" << m.direction_deg << "°"
           << " arc=" << m.arc_deg << "°";
        log(ms.str());
    }
}

// ── Logging ─────────────────────────────────────────────────────────

void PCGPreviewPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

}
