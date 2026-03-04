#include "ShipArchetypePanel.h"
#include <sstream>
#include <algorithm>
#include <fstream>
#include <filesystem>

namespace atlas::editor {

// ── Construction ───────────────────────────────────────────────────

ShipArchetypePanel::ShipArchetypePanel() {
    m_archetype = pcg::ShipArchetypeEngine::createDefault(
        pcg::HullClass::Frigate);
    m_pcgManager.initialize(42);
}

// ── Draw (stub) ────────────────────────────────────────────────────

void ShipArchetypePanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Ship Archetype", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    const float widgetW = b.w - 2.0f * pad;
    float y = b.y + headerH + pad;

    // Hull class combo box
    static const std::vector<std::string> hullItems = {
        "Frigate", "Destroyer", "Cruiser", "Battlecruiser", "Battleship", "Capital"
    };
    int hullIdx = static_cast<int>(m_archetype.hullClass);
    if (atlas::comboBox(ctx, "Hull Class", {b.x + pad, y, widgetW, rowH + pad},
                        hullItems, &hullIdx, &m_hullDropdownOpen)) {
        SelectHullClass(static_cast<pcg::HullClass>(hullIdx));
    }
    y += rowH + pad;

    // Hull control point count
    atlas::label(ctx, {b.x + pad, y},
        "Hull Control Points: " + std::to_string(m_archetype.hullShape.controlPoints.size()),
        ctx.theme().textSecondary);
    y += rowH + pad;

    // Variation bound sliders
    atlas::slider(ctx, "Shape Variation", {b.x + pad, y, widgetW, rowH + pad},
                  &m_archetype.shapeVariation, 0.0f, 1.0f, "%.2f");
    y += rowH + pad;
    atlas::slider(ctx, "Size Variation", {b.x + pad, y, widgetW, rowH + pad},
                  &m_archetype.sizeVariation, 0.0f, 1.0f, "%.2f");
    y += rowH + pad;
    atlas::slider(ctx, "Detail Variation", {b.x + pad, y, widgetW, rowH + pad},
                  &m_archetype.detailVariation, 0.0f, 1.0f, "%.2f");
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Room list
    atlas::label(ctx, {b.x + pad, y},
        "Rooms: " + std::to_string(m_archetype.rooms.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;
    {
        size_t count = m_archetype.rooms.size();
        size_t show = std::min(count, size_t(8));
        for (size_t i = 0; i < show; ++i) {
            const auto& room = m_archetype.rooms[i];
            atlas::label(ctx, {b.x + 2.0f * pad, y},
                "Room " + std::to_string(room.roomId) + ": " +
                pcg::interiorRoomTypeName(room.type),
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
        if (count > 8) {
            atlas::label(ctx, {b.x + 2.0f * pad, y},
                "... " + std::to_string(count - 8) + " more",
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
    }

    // Door count
    atlas::label(ctx, {b.x + pad, y},
        "Doors: " + std::to_string(m_archetype.doors.size()),
        ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Hardpoint list
    atlas::label(ctx, {b.x + pad, y},
        "Hardpoints: " + std::to_string(m_archetype.hardpoints.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;
    {
        size_t count = m_archetype.hardpoints.size();
        size_t show = std::min(count, size_t(8));
        for (size_t i = 0; i < show; ++i) {
            const auto& hp = m_archetype.hardpoints[i];
            atlas::label(ctx, {b.x + 2.0f * pad, y},
                "HP " + std::to_string(hp.hardpointId) + " [" + hp.groupTag + "]",
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
        if (count > 8) {
            atlas::label(ctx, {b.x + 2.0f * pad, y},
                "... " + std::to_string(count - 8) + " more",
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
    }

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Subsystem list
    atlas::label(ctx, {b.x + pad, y},
        "Subsystems: " + std::to_string(m_archetype.subsystems.size()),
        ctx.theme().textPrimary);
    y += rowH + pad;
    {
        size_t count = m_archetype.subsystems.size();
        size_t show = std::min(count, size_t(8));
        for (size_t i = 0; i < show; ++i) {
            const auto& ss = m_archetype.subsystems[i];
            atlas::label(ctx, {b.x + 2.0f * pad, y},
                std::string(pcg::ShipArchetypeEngine::subsystemTypeName(ss.type)),
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
        if (count > 8) {
            atlas::label(ctx, {b.x + 2.0f * pad, y},
                "... " + std::to_string(count - 8) + " more",
                ctx.theme().textSecondary);
            y += rowH + pad;
        }
    }

    // Module visual rule count
    atlas::label(ctx, {b.x + pad, y},
        "Module Visual Rules: " + std::to_string(m_archetype.moduleVisuals.size()),
        ctx.theme().textSecondary);
    y += rowH + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Generate Preview button
    const float btnW = 130.0f;
    if (atlas::button(ctx, "Generate Preview", {b.x + pad, y, btnW, rowH + pad})) {
        GeneratePreview();
    }
    y += rowH + pad + pad;

    // Save / Load buttons
    float halfW = (b.w - 3.0f * pad) * 0.5f;
    if (atlas::button(ctx, "Save", {b.x + pad, y, halfW, rowH + pad})) {
        SaveToFile();
    }
    if (atlas::button(ctx, "Load", {b.x + 2.0f * pad + halfW, y, halfW, rowH + pad})) {
        LoadFromFile();
    }
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, widgetW);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, widgetW, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Hull class selection ───────────────────────────────────────────

void ShipArchetypePanel::SelectHullClass(pcg::HullClass hull) {
    m_archetype = pcg::ShipArchetypeEngine::createDefault(hull);
    m_hasPreview = false;
    m_log.clear();
    log("Selected hull class: " +
        pcg::ShipGenerator::hullClassName(hull));
    log("  Rooms: " + std::to_string(m_archetype.rooms.size()));
    log("  Doors: " + std::to_string(m_archetype.doors.size()));
    log("  Hardpoints: " + std::to_string(m_archetype.hardpoints.size()));
    log("  Subsystem slots: " + std::to_string(m_archetype.subsystems.size()));
}

// ── Hull shape editing ─────────────────────────────────────────────

void ShipArchetypePanel::AddHullControlPoint(
        const pcg::ShapeControlPoint& cp) {
    m_archetype.hullShape.controlPoints.push_back(cp);
    log("Added hull control point at (" +
        std::to_string(cp.posX) + ", " +
        std::to_string(cp.posY) + ", " +
        std::to_string(cp.posZ) + ")");
}

bool ShipArchetypePanel::RemoveHullControlPoint(size_t index) {
    if (index >= m_archetype.hullShape.controlPoints.size()) return false;
    m_archetype.hullShape.controlPoints.erase(
        m_archetype.hullShape.controlPoints.begin() +
        static_cast<ptrdiff_t>(index));
    log("Removed hull control point " + std::to_string(index));
    return true;
}

bool ShipArchetypePanel::UpdateHullControlPoint(
        size_t index, const pcg::ShapeControlPoint& cp) {
    if (index >= m_archetype.hullShape.controlPoints.size()) return false;
    m_archetype.hullShape.controlPoints[index] = cp;
    log("Updated hull control point " + std::to_string(index));
    return true;
}

// ── Interior room editing ──────────────────────────────────────────

void ShipArchetypePanel::AddRoom(const pcg::InteriorRoom& room) {
    m_archetype.rooms.push_back(room);
    log("Added room " + std::to_string(room.roomId) + " (" +
        pcg::interiorRoomTypeName(room.type) + ")");
}

bool ShipArchetypePanel::RemoveRoom(int roomId) {
    auto it = std::remove_if(m_archetype.rooms.begin(),
                              m_archetype.rooms.end(),
        [roomId](const pcg::InteriorRoom& r) { return r.roomId == roomId; });
    if (it == m_archetype.rooms.end()) return false;
    m_archetype.rooms.erase(it, m_archetype.rooms.end());

    // Also remove doors referencing this room.
    m_archetype.doors.erase(
        std::remove_if(m_archetype.doors.begin(), m_archetype.doors.end(),
            [roomId](const pcg::DoorPlacement& d) {
                return d.fromRoomId == roomId || d.toRoomId == roomId;
            }),
        m_archetype.doors.end());

    log("Removed room " + std::to_string(roomId) +
        " (and associated doors)");
    return true;
}

// ── Door placement ─────────────────────────────────────────────────

void ShipArchetypePanel::AddDoor(const pcg::DoorPlacement& door) {
    m_archetype.doors.push_back(door);
    log("Added door " + std::to_string(door.doorId) +
        " (room " + std::to_string(door.fromRoomId) +
        " → room " + std::to_string(door.toRoomId) + ")" +
        (door.isAirlock ? " [airlock]" : ""));
}

bool ShipArchetypePanel::RemoveDoor(uint32_t doorId) {
    auto it = std::remove_if(m_archetype.doors.begin(),
                              m_archetype.doors.end(),
        [doorId](const pcg::DoorPlacement& d) { return d.doorId == doorId; });
    if (it == m_archetype.doors.end()) return false;
    m_archetype.doors.erase(it, m_archetype.doors.end());
    log("Removed door " + std::to_string(doorId));
    return true;
}

// ── Hardpoint editing ──────────────────────────────────────────────

void ShipArchetypePanel::AddHardpoint(const pcg::HardpointDefinition& hp) {
    m_archetype.hardpoints.push_back(hp);
    log("Added hardpoint " + std::to_string(hp.hardpointId) +
        " [" + hp.groupTag + "] " +
        (hp.isDorsal ? "dorsal" : "ventral"));
}

bool ShipArchetypePanel::RemoveHardpoint(uint32_t hardpointId) {
    auto it = std::remove_if(m_archetype.hardpoints.begin(),
                              m_archetype.hardpoints.end(),
        [hardpointId](const pcg::HardpointDefinition& hp) {
            return hp.hardpointId == hardpointId;
        });
    if (it == m_archetype.hardpoints.end()) return false;
    m_archetype.hardpoints.erase(it, m_archetype.hardpoints.end());
    log("Removed hardpoint " + std::to_string(hardpointId));
    return true;
}

bool ShipArchetypePanel::UpdateHardpoint(
        uint32_t hardpointId, const pcg::HardpointDefinition& hp) {
    for (auto& existing : m_archetype.hardpoints) {
        if (existing.hardpointId == hardpointId) {
            existing = hp;
            log("Updated hardpoint " + std::to_string(hardpointId));
            return true;
        }
    }
    return false;
}

// ── Subsystem slot editing ─────────────────────────────────────────

void ShipArchetypePanel::AddSubsystemSlot(const pcg::SubsystemSlot& slot) {
    m_archetype.subsystems.push_back(slot);
    log("Added subsystem slot: " +
        std::string(pcg::ShipArchetypeEngine::subsystemTypeName(slot.type)));
}

bool ShipArchetypePanel::AddSubsystemVariant(
        size_t slotIndex, const pcg::SubsystemVariant& variant) {
    if (slotIndex >= m_archetype.subsystems.size()) return false;
    m_archetype.subsystems[slotIndex].variants.push_back(variant);
    log("Added variant '" + variant.name + "' to subsystem slot " +
        std::to_string(slotIndex));
    return true;
}

bool ShipArchetypePanel::SetActiveVariant(size_t slotIndex,
                                            int variantIndex) {
    if (slotIndex >= m_archetype.subsystems.size()) return false;
    auto& slot = m_archetype.subsystems[slotIndex];
    if (variantIndex >= static_cast<int>(slot.variants.size())) return false;
    slot.activeVariant = variantIndex;
    std::string varName = (variantIndex >= 0)
        ? slot.variants[static_cast<size_t>(variantIndex)].name
        : "none";
    log("Set active variant for slot " + std::to_string(slotIndex) +
        ": " + varName);
    return true;
}

// ── Module visual rules ────────────────────────────────────────────

void ShipArchetypePanel::AddModuleVisualRule(
        const pcg::ModuleVisualRule& rule) {
    m_archetype.moduleVisuals.push_back(rule);
    log("Added module visual rule: " + rule.moduleCategory +
        " → " + rule.effectType);
}

bool ShipArchetypePanel::RemoveModuleVisualRule(size_t index) {
    if (index >= m_archetype.moduleVisuals.size()) return false;
    m_archetype.moduleVisuals.erase(
        m_archetype.moduleVisuals.begin() +
        static_cast<ptrdiff_t>(index));
    log("Removed module visual rule " + std::to_string(index));
    return true;
}

// ── Variation bounds ───────────────────────────────────────────────

void ShipArchetypePanel::SetVariationBounds(float shape, float size,
                                              float detail) {
    m_archetype.shapeVariation  = shape;
    m_archetype.sizeVariation   = size;
    m_archetype.detailVariation = detail;
    log("Set variation bounds: shape=" + std::to_string(shape) +
        " size=" + std::to_string(size) +
        " detail=" + std::to_string(detail));
}

// ── Preview generation ─────────────────────────────────────────────

void ShipArchetypePanel::GeneratePreview() {
    m_log.clear();

    if (!pcg::ShipArchetypeEngine::validate(m_archetype)) {
        log("ERROR: Archetype validation failed");
        m_hasPreview = false;
        return;
    }

    m_pcgManager.initialize(42);
    pcg::PCGContext ctx = m_pcgManager.makeRootContext(
        pcg::PCGDomain::Ship, 1, m_archetype.version);

    log("Generating preview for: " + m_archetype.name);
    m_preview = pcg::ShipArchetypeEngine::generateFromArchetype(
        ctx, m_archetype);
    m_hasPreview = true;

    std::ostringstream os;
    os << "Preview: " << m_preview.ship.shipName
       << " | Hull: " << pcg::ShipGenerator::hullClassName(
              m_preview.ship.hullClass)
       << " | Mass: " << m_preview.ship.mass
       << " | Turrets: " << m_preview.ship.turretSlots
       << " | Launchers: " << m_preview.ship.launcherSlots
       << " | Hardpoints: " << m_preview.hardpoints.size()
       << " | Doors: " << m_preview.doors.size()
       << " | Rooms: " << m_preview.interior.rooms.size()
       << " | Valid: " << (m_preview.valid ? "yes" : "NO");
    log(os.str());
}

void ShipArchetypePanel::GeneratePreviewWithSubsystems(
        const std::vector<int>& activeVariants) {
    GeneratePreview();
    if (!m_hasPreview) return;

    float preMass   = m_preview.ship.mass;
    float preShield = m_preview.ship.shieldHP;

    pcg::ShipArchetypeEngine::applySubsystems(
        m_preview, m_archetype, activeVariants);

    log("Applied subsystems:");
    for (size_t i = 0; i < activeVariants.size() &&
                        i < m_archetype.subsystems.size(); ++i) {
        int vi = activeVariants[i];
        if (vi >= 0 && vi < static_cast<int>(
                m_archetype.subsystems[i].variants.size())) {
            log("  " + std::string(pcg::ShipArchetypeEngine::subsystemTypeName(
                    m_archetype.subsystems[i].type)) + ": " +
                m_archetype.subsystems[i].variants[
                    static_cast<size_t>(vi)].name);
        }
    }

    std::ostringstream os;
    os << "After subsystems: Mass " << preMass << " → " << m_preview.ship.mass
       << " | Shield " << preShield << " → " << m_preview.ship.shieldHP;
    log(os.str());
}

void ShipArchetypePanel::GeneratePreviewWithModules(
        const std::vector<std::string>& fittedModules) {
    GeneratePreview();
    if (!m_hasPreview) return;

    pcg::ShipArchetypeEngine::applyModuleVisuals(
        m_preview, m_archetype, fittedModules);

    log("Applied module visuals: " + std::to_string(fittedModules.size()) +
        " modules");
    for (const auto& mod : fittedModules) {
        log("  Module: " + mod);
    }
}

// ── Serialisation ──────────────────────────────────────────────────

std::string ShipArchetypePanel::SaveToString() const {
    return pcg::ShipArchetypeEngine::serialize(m_archetype);
}

void ShipArchetypePanel::LoadFromString(const std::string& data) {
    m_archetype  = pcg::ShipArchetypeEngine::deserialize(data);
    m_hasPreview = false;
    m_log.clear();
    log("Loaded archetype: " + m_archetype.name);
}

bool ShipArchetypePanel::SaveToFile(const std::string& path) {
    std::filesystem::path fspath(path);
    if (fspath.has_parent_path()) {
        std::filesystem::create_directories(fspath.parent_path());
    }

    std::ofstream out(path);
    if (!out.is_open()) {
        log("ERROR: Could not open " + path + " for writing");
        return false;
    }

    out << SaveToString();
    out.close();
    log("Saved archetype to " + path);
    return true;
}

bool ShipArchetypePanel::LoadFromFile(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        log("File not found: " + path);
        return false;
    }

    std::ifstream in(path);
    if (!in.is_open()) {
        log("ERROR: Could not open " + path + " for reading");
        return false;
    }

    std::string data((std::istreambuf_iterator<char>(in)),
                      std::istreambuf_iterator<char>());
    in.close();
    LoadFromString(data);
    return true;
}

// ── Logging ────────────────────────────────────────────────────────

void ShipArchetypePanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

}
