#include "GalaxyMapPanel.h"
#include "../../cpp_server/include/pcg/galaxy_generator.h"

#include <string>
#include <cmath>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

GalaxyMapPanel::GalaxyMapPanel() {
    m_mgr.initialize(m_seed);
}

// ── Generation ───────────────────────────────────────────────────────

void GalaxyMapPanel::Generate() {
    m_mgr.initialize(m_seed);
    pcg::PCGContext ctx = m_mgr.makeRootContext(pcg::PCGDomain::Galaxy, 1, 1);

    m_galaxy = pcg::GalaxyGenerator::generate(ctx, m_systemCount);

    // Clear selection when the galaxy changes.
    m_selectedSystemId = 0;

    rebuildFilter();

    int cp = ChokepointCount();
    log("Generated galaxy: " + std::to_string(m_galaxy.total_systems)
        + " systems, " + std::to_string(cp) + " chokepoints"
        + " [HS=" + std::to_string(m_galaxy.highsec_count)
        + " LS=" + std::to_string(m_galaxy.lowsec_count)
        + " NS=" + std::to_string(m_galaxy.nullsec_count) + "]");
}

void GalaxyMapPanel::SetSeed(uint64_t seed) {
    m_seed = seed;
}

void GalaxyMapPanel::SetSystemCount(int count) {
    if (count < 10)  count = 10;
    if (count > 500) count = 500;
    m_systemCount = count;
}

// ── Filtering / selection ─────────────────────────────────────────────

void GalaxyMapPanel::SetSecurityFilter(int filter) {
    if (filter < 0 || filter > 3) filter = 0;
    m_securityFilter = filter;
    rebuildFilter();
}

void GalaxyMapPanel::SelectSystem(uint64_t systemId) {
    m_selectedSystemId = 0;
    if (systemId == 0) return;

    for (const auto& node : m_galaxy.nodes) {
        if (node.system_id == systemId) {
            m_selectedSystemId = systemId;
            return;
        }
    }
    // systemId not found — selection stays cleared.
}

void GalaxyMapPanel::ClearSelection() {
    m_selectedSystemId = 0;
}

const pcg::GalaxyNode* GalaxyMapPanel::GetSelectedNode() const {
    if (m_selectedSystemId == 0) return nullptr;
    for (const auto& node : m_galaxy.nodes) {
        if (node.system_id == m_selectedSystemId) {
            return &node;
        }
    }
    return nullptr;
}

// ── Statistics ────────────────────────────────────────────────────────

int GalaxyMapPanel::ChokepointCount() const {
    int n = 0;
    for (const auto& r : m_galaxy.routes) {
        if (r.is_chokepoint) ++n;
    }
    return n;
}

// ── Draw ──────────────────────────────────────────────────────────────

void GalaxyMapPanel::Draw() {
    // When running headless (no context) the draw call is a no-op.
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Galaxy Map", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    if (m_galaxy.valid) {
        std::string summary =
            "Systems: " + std::to_string(m_galaxy.total_systems)
            + "  HS:" + std::to_string(m_galaxy.highsec_count)
            + "  LS:" + std::to_string(m_galaxy.lowsec_count)
            + "  NS:" + std::to_string(m_galaxy.nullsec_count)
            + "  Chokepoints:" + std::to_string(ChokepointCount());
        atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    } else {
        atlas::label(ctx, {b.x + pad, y}, "No galaxy generated.",
                     ctx.theme().textSecondary);
    }
    y += rowH + pad;

    // Selected system info.
    const pcg::GalaxyNode* sel = GetSelectedNode();
    if (sel) {
        std::string info =
            "Selected: id=" + std::to_string(sel->system_id)
            + " zone=" + pcg::GalaxyGenerator::securityZoneName(sel->security)
            + " sec=" + std::to_string(static_cast<int>(sel->security_level * 10))
            + "/10"
            + " neighbours=" + std::to_string(sel->neighbor_ids.size())
            + " const=" + std::to_string(sel->constellation_id)
            + " region=" + std::to_string(sel->region_id);
        atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textPrimary);
        y += rowH + pad;
    }

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area.
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void GalaxyMapPanel::rebuildFilter() {
    m_filtered.clear();
    for (size_t i = 0; i < m_galaxy.nodes.size(); ++i) {
        const auto& n = m_galaxy.nodes[i];
        bool pass = false;
        switch (m_securityFilter) {
            case 0: pass = true; break;
            case 1: pass = (n.security == pcg::SecurityZone::HighSec); break;
            case 2: pass = (n.security == pcg::SecurityZone::LowSec);  break;
            case 3: pass = (n.security == pcg::SecurityZone::NullSec); break;
            default: pass = true; break;
        }
        if (pass) m_filtered.push_back(i);
    }
}

void GalaxyMapPanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor

