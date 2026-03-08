#pragma once
#include "../ui/EditorPanel.h"
#include "../../cpp_client/include/ui/atlas/atlas_widgets.h"
#include <string>
#include <vector>
#include <cstdint>

namespace atlas::editor {

/**
 * @brief A trade route entry for designer editing.
 */
struct TradeRouteEntry {
    uint32_t routeId = 0;
    std::string routeName;
    std::string origin;
    std::string destination;
    std::string commodity;
    float baseVolume    = 0.0f;
    float baseRevenue   = 0.0f;
    float congestion    = 0.0f;   ///< 0.0 = clear, 1.0 = fully congested
    bool  enabled       = true;
};

/**
 * TradeRoutePanel — Trade route visualization and editing tool.
 *
 * Designers can:
 *   - Create trade routes between systems with commodity assignments.
 *   - Set base volume and revenue per route for economy tuning.
 *   - Adjust congestion levels to simulate bottlenecks.
 *   - Enable/disable routes for A/B testing.
 *   - View route summaries (total volume, revenue, route count).
 *   - Export route definitions to JSON for the economy data pipeline.
 *
 * Headless-safe: Draw() is a no-op when no AtlasContext is set.
 */
class TradeRoutePanel : public EditorPanel {
public:
    TradeRoutePanel();
    ~TradeRoutePanel() override = default;

    const char* Name() const override { return "Trade Route Editor"; }
    void Draw() override;

    // ── Route management ─────────────────────────────────────────

    int AddRoute(const std::string& routeName, const std::string& origin,
                 const std::string& destination, const std::string& commodity);
    bool RemoveRoute(uint32_t routeId);
    int RouteCount() const { return static_cast<int>(m_routes.size()); }
    const std::vector<TradeRouteEntry>& Routes() const { return m_routes; }

    // ── Route properties ─────────────────────────────────────────

    bool SetRouteVolume(uint32_t routeId, float volume);
    bool SetRouteRevenue(uint32_t routeId, float revenue);
    bool SetRouteCongestion(uint32_t routeId, float congestion);
    bool ToggleRoute(uint32_t routeId, bool enabled);

    float GetRouteVolume(uint32_t routeId) const;
    float GetRouteRevenue(uint32_t routeId) const;
    float GetRouteCongestion(uint32_t routeId) const;

    int EnabledRouteCount() const;

    // ── Aggregate stats ──────────────────────────────────────────

    float TotalVolume() const;
    float TotalRevenue() const;

    // ── Export ────────────────────────────────────────────────────

    std::string ExportJSON() const;

    // ── Log ──────────────────────────────────────────────────────

    const std::vector<std::string>& Log() const { return m_log; }

private:
    std::vector<TradeRouteEntry> m_routes;

    static constexpr int kMaxRoutes = 30;
    uint32_t m_nextRouteId = 1;

    std::vector<std::string> m_log;
    atlas::PanelState m_panelState;
    float m_scrollOffset = 0.0f;

    void log(const std::string& msg);
    TradeRouteEntry* findRoute(uint32_t routeId);
    const TradeRouteEntry* findRouteConst(uint32_t routeId) const;
};

} // namespace atlas::editor
