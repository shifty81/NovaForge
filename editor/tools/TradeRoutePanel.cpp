#include "TradeRoutePanel.h"

#include <algorithm>
#include <sstream>

namespace atlas::editor {

// ── Construction ─────────────────────────────────────────────────────

TradeRoutePanel::TradeRoutePanel() = default;

// ── Internal helpers ─────────────────────────────────────────────────

TradeRouteEntry* TradeRoutePanel::findRoute(uint32_t routeId) {
    for (auto& r : m_routes) {
        if (r.routeId == routeId) return &r;
    }
    return nullptr;
}

const TradeRouteEntry* TradeRoutePanel::findRouteConst(uint32_t routeId) const {
    for (const auto& r : m_routes) {
        if (r.routeId == routeId) return &r;
    }
    return nullptr;
}

// ── Route management ─────────────────────────────────────────────────

int TradeRoutePanel::AddRoute(const std::string& routeName,
                               const std::string& origin,
                               const std::string& destination,
                               const std::string& commodity) {
    if (static_cast<int>(m_routes.size()) >= kMaxRoutes) return -1;
    if (routeName.empty() || origin.empty() || destination.empty() || commodity.empty()) return -1;

    TradeRouteEntry entry;
    entry.routeId     = m_nextRouteId++;
    entry.routeName   = routeName;
    entry.origin      = origin;
    entry.destination = destination;
    entry.commodity   = commodity;
    m_routes.push_back(entry);

    log("Added route #" + std::to_string(entry.routeId)
        + " \"" + routeName + "\" " + origin + " → " + destination);
    return static_cast<int>(entry.routeId);
}

bool TradeRoutePanel::RemoveRoute(uint32_t routeId) {
    auto it = std::find_if(m_routes.begin(), m_routes.end(),
        [routeId](const TradeRouteEntry& r) { return r.routeId == routeId; });
    if (it == m_routes.end()) return false;
    log("Removed route #" + std::to_string(routeId));
    m_routes.erase(it);
    return true;
}

// ── Route properties ─────────────────────────────────────────────────

bool TradeRoutePanel::SetRouteVolume(uint32_t routeId, float volume) {
    auto* r = findRoute(routeId);
    if (!r) return false;
    r->baseVolume = std::max(0.0f, volume);
    return true;
}

bool TradeRoutePanel::SetRouteRevenue(uint32_t routeId, float revenue) {
    auto* r = findRoute(routeId);
    if (!r) return false;
    r->baseRevenue = std::max(0.0f, revenue);
    return true;
}

bool TradeRoutePanel::SetRouteCongestion(uint32_t routeId, float congestion) {
    auto* r = findRoute(routeId);
    if (!r) return false;
    r->congestion = std::max(0.0f, std::min(1.0f, congestion));
    return true;
}

bool TradeRoutePanel::ToggleRoute(uint32_t routeId, bool enabled) {
    auto* r = findRoute(routeId);
    if (!r) return false;
    if (r->enabled == enabled) return false;  // already in desired state
    r->enabled = enabled;
    log("Route #" + std::to_string(routeId) + " "
        + (enabled ? "enabled" : "disabled"));
    return true;
}

float TradeRoutePanel::GetRouteVolume(uint32_t routeId) const {
    const auto* r = findRouteConst(routeId);
    return r ? r->baseVolume : 0.0f;
}

float TradeRoutePanel::GetRouteRevenue(uint32_t routeId) const {
    const auto* r = findRouteConst(routeId);
    return r ? r->baseRevenue : 0.0f;
}

float TradeRoutePanel::GetRouteCongestion(uint32_t routeId) const {
    const auto* r = findRouteConst(routeId);
    return r ? r->congestion : 0.0f;
}

int TradeRoutePanel::EnabledRouteCount() const {
    int count = 0;
    for (const auto& r : m_routes) {
        if (r.enabled) ++count;
    }
    return count;
}

// ── Aggregate stats ──────────────────────────────────────────────────

float TradeRoutePanel::TotalVolume() const {
    float total = 0.0f;
    for (const auto& r : m_routes) {
        if (r.enabled) total += r.baseVolume;
    }
    return total;
}

float TradeRoutePanel::TotalRevenue() const {
    float total = 0.0f;
    for (const auto& r : m_routes) {
        if (r.enabled) total += r.baseRevenue;
    }
    return total;
}

// ── Export ────────────────────────────────────────────────────────────

std::string TradeRoutePanel::ExportJSON() const {
    std::ostringstream os;
    os << "{\n";
    os << "  \"routes\": [\n";
    for (size_t i = 0; i < m_routes.size(); ++i) {
        const auto& r = m_routes[i];
        os << "    {\"id\": " << r.routeId
           << ", \"name\": \"" << r.routeName << "\""
           << ", \"origin\": \"" << r.origin << "\""
           << ", \"destination\": \"" << r.destination << "\""
           << ", \"commodity\": \"" << r.commodity << "\""
           << ", \"volume\": " << r.baseVolume
           << ", \"revenue\": " << r.baseRevenue
           << ", \"congestion\": " << r.congestion
           << ", \"enabled\": " << (r.enabled ? "true" : "false") << "}";
        if (i + 1 < m_routes.size()) os << ",";
        os << "\n";
    }
    os << "  ]\n";
    os << "}";
    return os.str();
}

// ── Draw ──────────────────────────────────────────────────────────────

void TradeRoutePanel::Draw() {
    if (!GetContext()) return;

    auto& ctx = *GetContext();
    ApplyDockBounds(m_panelState);
    if (!atlas::panelBeginStateful(ctx, "Trade Route Editor", m_panelState)) {
        atlas::panelEnd(ctx);
        return;
    }

    const float pad    = ctx.theme().padding;
    const float rowH   = ctx.theme().rowHeight;
    const atlas::Rect& b = m_panelState.bounds;
    const float headerH  = ctx.theme().headerHeight;
    float y = b.y + headerH + pad;

    // Summary
    std::string summary = "Routes: " + std::to_string(RouteCount())
        + "  Enabled: " + std::to_string(EnabledRouteCount())
        + "  Volume: " + std::to_string(static_cast<int>(TotalVolume()))
        + "  Revenue: " + std::to_string(static_cast<int>(TotalRevenue()));
    atlas::label(ctx, {b.x + pad, y}, summary, ctx.theme().textPrimary);
    y += rowH + pad;

    // Route listing
    for (int i = 0; i < RouteCount() && i < 12; ++i) {
        const auto& r = m_routes[i];
        std::string info = "#" + std::to_string(r.routeId) + " " + r.routeName
            + " " + r.origin + "→" + r.destination
            + " [" + r.commodity + "]"
            + (r.enabled ? "" : " (disabled)");
        atlas::label(ctx, {b.x + pad, y}, info, ctx.theme().textPrimary);
        y += rowH;
    }
    y += pad;

    atlas::separator(ctx, {b.x + pad, y}, b.w - 2.0f * pad);
    y += pad;

    // Log area
    atlas::Rect logRect{b.x + pad, y, b.w - 2.0f * pad, b.y + b.h - y - pad};
    atlas::combatLogWidget(ctx, logRect, m_log, m_scrollOffset);

    atlas::panelEnd(ctx);
}

// ── Helpers ───────────────────────────────────────────────────────────

void TradeRoutePanel::log(const std::string& msg) {
    m_log.push_back(msg);
}

} // namespace atlas::editor
