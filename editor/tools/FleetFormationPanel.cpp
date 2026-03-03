#include "FleetFormationPanel.h"
#include <cmath>
#include <sstream>
#include <algorithm>

namespace atlas::editor {

// ── Construction ────────────────────────────────────────────────────

FleetFormationPanel::FleetFormationPanel() {
    rebuildSlots();
    log("[Formation] Initialized with Arrow × 5");
}

// ── Draw (headless-safe) ────────────────────────────────────────────

void FleetFormationPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Fleet Formation", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad     = ctx.theme().padding;
    const float rowH    = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Formation type label
    atlas::label(ctx, {b.x + pad, y},
        std::string("Formation: ") + FormationTypeName(m_formationType),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Fleet size
    atlas::label(ctx, {b.x + pad, y},
        "Fleet Size: " + std::to_string(m_fleetSize),
        ctx.theme().textPrimary);
    y += rowH + pad;

    // Spacing
    atlas::label(ctx, {b.x + pad, y},
        "Spacing: " + std::to_string(static_cast<int>(m_spacing)) + " m",
        ctx.theme().textSecondary);
    y += rowH + pad;

    // Bounding radius
    atlas::label(ctx, {b.x + pad, y},
        "Radius: " + std::to_string(static_cast<int>(BoundingRadius())) + " m",
        ctx.theme().textSecondary);
    y += rowH + pad + pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Slot list
    for (size_t i = 0; i < m_slots.size(); ++i) {
        if (y + rowH > b.y + b.h - pad) break;
        const auto& s = m_slots[i];
        std::string label = "#" + std::to_string(s.slotIndex) + " "
            + s.shipName + " [" + s.role + "]";
        if (static_cast<int>(i) == m_selectedSlot)
            label = "> " + label;

        atlas::Rect row{b.x + pad, y, b.w - 2.0f * pad, rowH};
        if (atlas::button(ctx, label.c_str(), row)) {
            SelectSlot(static_cast<int>(i));
        }
        y += rowH + pad;
    }

    atlas::panelEnd(ctx);
}

// ── Formation type ──────────────────────────────────────────────────

void FleetFormationPanel::SetFormationType(FormationType type) {
    m_formationType = type;
    ComputeOffsets();
    log("[Formation] Type changed to " + std::string(FormationTypeName(type)));
}

const char* FleetFormationPanel::FormationTypeName(FormationType type) {
    switch (type) {
        case FormationType::None:    return "None";
        case FormationType::Arrow:   return "Arrow";
        case FormationType::Line:    return "Line";
        case FormationType::Wedge:   return "Wedge";
        case FormationType::Spread:  return "Spread";
        case FormationType::Diamond: return "Diamond";
    }
    return "Unknown";
}

// ── Fleet size / spacing ────────────────────────────────────────────

void FleetFormationPanel::SetFleetSize(int size) {
    m_fleetSize = std::max(1, std::min(size, 256));
    rebuildSlots();
    log("[Formation] Fleet size set to " + std::to_string(m_fleetSize));
}

void FleetFormationPanel::SetSpacing(float metres) {
    m_spacing = std::max(10.0f, std::min(metres, 10000.0f));
    ComputeOffsets();
    log("[Formation] Spacing set to " + std::to_string(static_cast<int>(m_spacing)) + " m");
}

// ── Slot management ─────────────────────────────────────────────────

void FleetFormationPanel::rebuildSlots() {
    m_slots.resize(static_cast<size_t>(m_fleetSize));
    for (int i = 0; i < m_fleetSize; ++i) {
        auto& s = m_slots[static_cast<size_t>(i)];
        s.slotIndex = i;
        if (s.shipName.empty()) {
            s.shipName = "Ship_" + std::to_string(i);
        }
        if (s.role.empty()) {
            s.role = (i == 0) ? "command" : "dps";
        }
    }
    ComputeOffsets();
}

void FleetFormationPanel::ComputeOffsets() {
    for (auto& slot : m_slots) {
        slot.offsetX = 0.0f;
        slot.offsetY = 0.0f;
        slot.offsetZ = 0.0f;

        if (slot.slotIndex == 0) continue; // commander stays at origin

        switch (m_formationType) {
            case FormationType::None:    break;
            case FormationType::Arrow:   computeArrow(slot);   break;
            case FormationType::Line:    computeLine(slot);     break;
            case FormationType::Wedge:   computeWedge(slot);    break;
            case FormationType::Spread:  computeSpread(slot);   break;
            case FormationType::Diamond: computeDiamond(slot);  break;
        }

        // Apply per-slot spacing modifier
        slot.offsetX *= slot.spacingModifier;
        slot.offsetY *= slot.spacingModifier;
        slot.offsetZ *= slot.spacingModifier;
    }
}

// ── Formation offset algorithms (mirror FleetFormationSystem logic) ─

void FleetFormationPanel::computeArrow(FormationSlot& slot) const {
    int i = slot.slotIndex;
    int side = (i % 2 == 1) ? -1 : 1;
    int row  = (i + 1) / 2;
    slot.offsetX = static_cast<float>(side * row) * m_spacing;
    slot.offsetZ = static_cast<float>(-row) * m_spacing;
}

void FleetFormationPanel::computeLine(FormationSlot& slot) const {
    int i = slot.slotIndex;
    int side = (i % 2 == 1) ? -1 : 1;
    int col  = (i + 1) / 2;
    slot.offsetX = static_cast<float>(side * col) * m_spacing;
}

void FleetFormationPanel::computeWedge(FormationSlot& slot) const {
    int i = slot.slotIndex;
    int side = (i % 2 == 1) ? -1 : 1;
    int row  = (i + 1) / 2;
    slot.offsetX = static_cast<float>(side * row) * m_spacing;
    slot.offsetZ = static_cast<float>(-row) * m_spacing * 0.5f;
}

void FleetFormationPanel::computeSpread(FormationSlot& slot) const {
    int i = slot.slotIndex;
    float angle = static_cast<float>(i) * (2.0f * 3.14159265f / static_cast<float>(m_fleetSize));
    float radius = m_spacing;
    slot.offsetX = std::cos(angle) * radius;
    slot.offsetZ = std::sin(angle) * radius;
}

void FleetFormationPanel::computeDiamond(FormationSlot& slot) const {
    int i = slot.slotIndex;
    // Place slots in concentric diamond rings.
    int ring = 0;
    int consumed = 1; // slot 0 is the centre
    while (consumed + (ring + 1) * 4 <= i) {
        consumed += (ring + 1) * 4;
        ++ring;
    }
    int posInRing = i - consumed;
    int perimSlots = (ring + 1) * 4;
    float frac = static_cast<float>(posInRing) / static_cast<float>(perimSlots);
    float angle = frac * 2.0f * 3.14159265f;
    float radius = static_cast<float>(ring + 1) * m_spacing;
    slot.offsetX = std::cos(angle) * radius;
    slot.offsetZ = std::sin(angle) * radius;
}

// ── Import from PCG ─────────────────────────────────────────────────

void FleetFormationPanel::ImportFleet(
    const pcg::GeneratedFleetCompositionResult& fleet) {
    m_fleetSize = static_cast<int>(fleet.ships.size());
    m_slots.clear();
    m_slots.resize(fleet.ships.size());

    for (size_t i = 0; i < fleet.ships.size(); ++i) {
        auto& s = m_slots[i];
        s.slotIndex = static_cast<int>(i);
        s.shipName  = fleet.ships[i].shipName;
        s.role      = fleet.ships[i].role;
        s.hullClass = fleet.ships[i].hullClass;
    }

    ComputeOffsets();

    std::ostringstream oss;
    oss << "[Formation] Imported fleet '" << fleet.doctrineName
        << "' — " << fleet.ships.size() << " ships ("
        << fleet.capitalCount << " capital, "
        << fleet.subcapCount << " subcap)";
    log(oss.str());
}

// ── Selection ───────────────────────────────────────────────────────

void FleetFormationPanel::SelectSlot(int slotIndex) {
    if (slotIndex >= 0 && slotIndex < static_cast<int>(m_slots.size())) {
        m_selectedSlot = slotIndex;
    }
}

void FleetFormationPanel::ClearSelection() {
    m_selectedSlot = -1;
}

// ── Aggregate stats ─────────────────────────────────────────────────

float FleetFormationPanel::BoundingRadius() const {
    float maxDist = 0.0f;
    for (const auto& s : m_slots) {
        float d = std::sqrt(s.offsetX * s.offsetX
                          + s.offsetY * s.offsetY
                          + s.offsetZ * s.offsetZ);
        if (d > maxDist) maxDist = d;
    }
    return maxDist;
}

void FleetFormationPanel::CentreOfMass(float& cx, float& cy, float& cz) const {
    cx = cy = cz = 0.0f;
    if (m_slots.empty()) return;
    for (const auto& s : m_slots) {
        cx += s.offsetX;
        cy += s.offsetY;
        cz += s.offsetZ;
    }
    float n = static_cast<float>(m_slots.size());
    cx /= n;
    cy /= n;
    cz /= n;
}

// ── Private helpers ─────────────────────────────────────────────────

void FleetFormationPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
