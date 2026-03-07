#include "SolarSystemEditorPanel.h"
#include "../../cpp_server/include/pcg/star_system_generator.h"

#include <string>
#include <cmath>
#include <algorithm>
#include <sstream>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

SolarSystemEditorPanel::SolarSystemEditorPanel() {
    m_mgr.initialize(m_seed);
}

// ── Generation ───────────────────────────────────────────────────────

void SolarSystemEditorPanel::Generate() {
    m_mgr.initialize(m_seed);
    pcg::PCGContext ctx = m_mgr.makeRootContext(pcg::PCGDomain::Galaxy, 1, 1);

    if (m_starClassOverride >= 0 &&
        m_starClassOverride < pcg::STAR_CLASS_COUNT) {
        m_system = pcg::StarSystemGenerator::generate(
            ctx, m_securityLevel,
            static_cast<pcg::StarClass>(m_starClassOverride));
    } else {
        m_system = pcg::StarSystemGenerator::generate(ctx, m_securityLevel);
    }

    m_nextId = 10000;
    recountBodies();

    log("Generated system: star="
        + pcg::StarSystemGenerator::starClassName(m_system.star.starClass)
        + " orbits=" + std::to_string(m_system.orbitSlots.size())
        + " stations=" + std::to_string(m_system.stations.size())
        + " gates=" + std::to_string(m_system.gates.size())
        + " sec=" + std::to_string(static_cast<int>(m_securityLevel * 10))
        + "/10");
}

void SolarSystemEditorPanel::SetSeed(uint64_t seed) {
    m_seed = seed;
}

void SolarSystemEditorPanel::SetSecurityLevel(float sec) {
    m_securityLevel = std::max(0.0f, std::min(1.0f, sec));
}

void SolarSystemEditorPanel::SetStarClassOverride(int starClass) {
    m_starClassOverride = starClass;
}

// ── Orbit editing ────────────────────────────────────────────────────

int SolarSystemEditorPanel::AddOrbitSlot(pcg::OrbitSlotType type,
                                         pcg::SystemPlanetType planet,
                                         float radiusAU) {
    pcg::OrbitSlot slot;
    slot.orbitIndex = OrbitSlotCount();
    slot.orbitRadius = std::max(0.1f, radiusAU);
    slot.type = type;
    slot.planetType = planet;
    m_system.orbitSlots.push_back(slot);
    recountBodies();
    log("Added orbit slot #" + std::to_string(slot.orbitIndex)
        + " radius=" + std::to_string(radiusAU) + " AU");
    return slot.orbitIndex;
}

bool SolarSystemEditorPanel::RemoveOrbitSlot(int orbitIndex) {
    if (orbitIndex < 0 || orbitIndex >= OrbitSlotCount()) return false;
    m_system.orbitSlots.erase(m_system.orbitSlots.begin() + orbitIndex);
    // Re-index remaining slots.
    for (int i = 0; i < static_cast<int>(m_system.orbitSlots.size()); ++i) {
        m_system.orbitSlots[i].orbitIndex = i;
    }
    recountBodies();
    log("Removed orbit slot #" + std::to_string(orbitIndex));
    return true;
}

// ── Station editing ──────────────────────────────────────────────────

bool SolarSystemEditorPanel::AddStation(int orbitIndex,
                                        const std::string& faction) {
    if (orbitIndex < 0 || orbitIndex >= OrbitSlotCount()) return false;
    pcg::SystemStation st;
    st.stationId = nextId();
    st.orbitIndex = orbitIndex;
    st.faction = faction;
    m_system.stations.push_back(st);
    log("Added station id=" + std::to_string(st.stationId)
        + " orbit=" + std::to_string(orbitIndex)
        + " faction=" + faction);
    return true;
}

bool SolarSystemEditorPanel::RemoveStation(uint64_t stationId) {
    auto it = std::find_if(m_system.stations.begin(), m_system.stations.end(),
        [stationId](const pcg::SystemStation& s) {
            return s.stationId == stationId;
        });
    if (it == m_system.stations.end()) return false;
    log("Removed station id=" + std::to_string(stationId));
    m_system.stations.erase(it);
    return true;
}

// ── Gate editing ─────────────────────────────────────────────────────

bool SolarSystemEditorPanel::AddGate(int orbitIndex,
                                     uint64_t destinationSystemId) {
    if (orbitIndex < 0 || orbitIndex >= OrbitSlotCount()) return false;
    pcg::JumpGate gate;
    gate.gateId = nextId();
    gate.orbitIndex = orbitIndex;
    gate.destinationSystemId = destinationSystemId;
    m_system.gates.push_back(gate);
    log("Added gate id=" + std::to_string(gate.gateId)
        + " orbit=" + std::to_string(orbitIndex)
        + " dest=" + std::to_string(destinationSystemId));
    return true;
}

bool SolarSystemEditorPanel::RemoveGate(uint64_t gateId) {
    auto it = std::find_if(m_system.gates.begin(), m_system.gates.end(),
        [gateId](const pcg::JumpGate& g) {
            return g.gateId == gateId;
        });
    if (it == m_system.gates.end()) return false;
    log("Removed gate id=" + std::to_string(gateId));
    m_system.gates.erase(it);
    return true;
}

// ── Export ────────────────────────────────────────────────────────────

std::string SolarSystemEditorPanel::ExportJSON() const {
    if (!m_system.valid) return "{}";

    std::ostringstream os;
    os << "{\n";
    os << "  \"systemId\": " << m_system.systemId << ",\n";
    os << "  \"seed\": " << m_system.seed << ",\n";
    os << "  \"securityLevel\": " << m_system.securityLevel << ",\n";
    os << "  \"star\": {\n";
    os << "    \"class\": \"" << pcg::StarSystemGenerator::starClassName(m_system.star.starClass) << "\",\n";
    os << "    \"luminosity\": " << m_system.star.luminosity << ",\n";
    os << "    \"temperature\": " << m_system.star.temperature << ",\n";
    os << "    \"radius\": " << m_system.star.radius << "\n";
    os << "  },\n";

    os << "  \"orbits\": [\n";
    for (size_t i = 0; i < m_system.orbitSlots.size(); ++i) {
        const auto& o = m_system.orbitSlots[i];
        os << "    {\"index\": " << o.orbitIndex
           << ", \"radius\": " << o.orbitRadius
           << ", \"type\": " << static_cast<int>(o.type)
           << ", \"planetType\": " << static_cast<int>(o.planetType) << "}";
        if (i + 1 < m_system.orbitSlots.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";

    os << "  \"stations\": [\n";
    for (size_t i = 0; i < m_system.stations.size(); ++i) {
        const auto& s = m_system.stations[i];
        os << "    {\"id\": " << s.stationId
           << ", \"orbit\": " << s.orbitIndex
           << ", \"faction\": \"" << s.faction << "\"}";
        if (i + 1 < m_system.stations.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";

    os << "  \"gates\": [\n";
    for (size_t i = 0; i < m_system.gates.size(); ++i) {
        const auto& g = m_system.gates[i];
        os << "    {\"id\": " << g.gateId
           << ", \"orbit\": " << g.orbitIndex
           << ", \"destination\": " << g.destinationSystemId << "}";
        if (i + 1 < m_system.gates.size()) os << ",";
        os << "\n";
    }
    os << "  ],\n";

    os << "  \"totalPlanets\": " << m_system.totalPlanets << ",\n";
    os << "  \"totalBelts\": " << m_system.totalBelts << "\n";
    os << "}";

    return os.str();
}

// ── Draw ──────────────────────────────────────────────────────────────

void SolarSystemEditorPanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Solar System Editor", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    if (m_system.valid) {
        std::string summary =
            "Star: " + pcg::StarSystemGenerator::starClassName(m_system.star.starClass)
            + "  Orbits:" + std::to_string(OrbitSlotCount())
            + "  Planets:" + std::to_string(m_system.totalPlanets)
            + "  Belts:" + std::to_string(m_system.totalBelts)
            + "  Stations:" + std::to_string(StationCount())
            + "  Gates:" + std::to_string(GateCount())
            + "  Sec:" + std::to_string(static_cast<int>(m_system.securityLevel * 10))
            + "/10";
        atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    } else {
        atlas::label(ctx, {b.x + pad, y}, "No system generated. Click Generate.",
                     ctx.theme().textSecondary);
    }
    y += rowH + pad;

    // Orbit slot listing.
    static constexpr int MAX_DISPLAYED_ORBIT_SLOTS = 12;
    if (m_system.valid) {
        for (int i = 0; i < OrbitSlotCount() && i < MAX_DISPLAYED_ORBIT_SLOTS; ++i) {
            const auto& slot = m_system.orbitSlots[i];
            std::string slotInfo =
                "#" + std::to_string(i) + "  "
                + std::to_string(slot.orbitRadius) + " AU  type="
                + std::to_string(static_cast<int>(slot.type));
            atlas::label(ctx, {b.x + pad, y}, slotInfo, ctx.theme().textPrimary);
            y += rowH;
        }
        y += pad;
    }

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area.
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void SolarSystemEditorPanel::recountBodies() {
    int planets = 0, belts = 0;
    for (const auto& slot : m_system.orbitSlots) {
        if (slot.type == pcg::OrbitSlotType::Planet) ++planets;
        if (slot.type == pcg::OrbitSlotType::Belt)   ++belts;
    }
    m_system.totalPlanets = planets;
    m_system.totalBelts   = belts;
}

uint64_t SolarSystemEditorPanel::nextId() {
    return ++m_nextId;
}

void SolarSystemEditorPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
