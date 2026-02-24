#include "PCGPreviewPanel.h"
#include <sstream>

namespace atlas::editor {

// ── Helpers ─────────────────────────────────────────────────────────

static const char* previewModeName(PCGPreviewMode mode) {
    switch (mode) {
        case PCGPreviewMode::Ship:     return "Ship";
        case PCGPreviewMode::Station:  return "Station";
        case PCGPreviewMode::Interior: return "Interior";
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
    // Stub: In a real implementation, this would render via Atlas UI:
    //   • Mode selector (Ship / Station / Interior)
    //   • Seed input field + Randomize button
    //   • Domain-specific parameter widgets
    //   • Generate button
    //   • Preview area showing the generated result
    //   • Log output
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
        case PCGPreviewMode::Ship:     generateShip();     break;
        case PCGPreviewMode::Station:  generateStation();  break;
        case PCGPreviewMode::Interior: generateInterior(); break;
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
    m_shipPreview     = ShipPreview{};
    m_stationPreview  = StationPreview{};
    m_interiorPreview = InteriorPreview{};
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

// ── Logging ─────────────────────────────────────────────────────────

void PCGPreviewPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

}
