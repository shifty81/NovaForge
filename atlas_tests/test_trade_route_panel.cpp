/**
 * Tests for TradeRoutePanel:
 *   - Construction & defaults
 *   - Add/remove routes
 *   - Route properties (volume, revenue, congestion)
 *   - Toggle enable/disable
 *   - Aggregate stats
 *   - Max routes limit
 *   - Export JSON
 *   - Headless draw
 */

#include <cassert>
#include <string>
#include "../editor/tools/TradeRoutePanel.h"

using namespace atlas::editor;

void test_trade_route_defaults() {
    TradeRoutePanel panel;
    assert(std::string(panel.Name()) == "Trade Route Editor");
    assert(panel.RouteCount() == 0);
    assert(panel.EnabledRouteCount() == 0);
    assert(panel.TotalVolume() < 0.01f);
    assert(panel.TotalRevenue() < 0.01f);
}

void test_trade_route_add() {
    TradeRoutePanel panel;
    int id1 = panel.AddRoute("Jita-Amarr", "Jita", "Amarr", "Tritanium");
    assert(id1 > 0);
    int id2 = panel.AddRoute("Amarr-Dodixie", "Amarr", "Dodixie", "Pyerite");
    assert(id2 > 0);
    assert(panel.RouteCount() == 2);
    assert(panel.EnabledRouteCount() == 2);
}

void test_trade_route_add_validation() {
    TradeRoutePanel panel;
    assert(panel.AddRoute("", "Jita", "Amarr", "Tritanium") == -1);
    assert(panel.AddRoute("Route", "", "Amarr", "Tritanium") == -1);
    assert(panel.AddRoute("Route", "Jita", "", "Tritanium") == -1);
    assert(panel.AddRoute("Route", "Jita", "Amarr", "") == -1);
    assert(panel.RouteCount() == 0);
}

void test_trade_route_remove() {
    TradeRoutePanel panel;
    int id = panel.AddRoute("Jita-Amarr", "Jita", "Amarr", "Tritanium");
    assert(panel.RemoveRoute(static_cast<uint32_t>(id)));
    assert(panel.RouteCount() == 0);
    assert(!panel.RemoveRoute(static_cast<uint32_t>(id))); // double remove
}

void test_trade_route_volume() {
    TradeRoutePanel panel;
    int id = panel.AddRoute("Route1", "Jita", "Amarr", "Tritanium");
    assert(panel.SetRouteVolume(static_cast<uint32_t>(id), 5000.0f));
    assert(panel.GetRouteVolume(static_cast<uint32_t>(id)) > 4999.0f);
    assert(panel.GetRouteVolume(static_cast<uint32_t>(id)) < 5001.0f);
    assert(!panel.SetRouteVolume(999, 100.0f)); // missing route
}

void test_trade_route_revenue() {
    TradeRoutePanel panel;
    int id = panel.AddRoute("Route1", "Jita", "Amarr", "Tritanium");
    assert(panel.SetRouteRevenue(static_cast<uint32_t>(id), 10000.0f));
    assert(panel.GetRouteRevenue(static_cast<uint32_t>(id)) > 9999.0f);
    assert(panel.GetRouteRevenue(static_cast<uint32_t>(id)) < 10001.0f);
}

void test_trade_route_congestion() {
    TradeRoutePanel panel;
    int id = panel.AddRoute("Route1", "Jita", "Amarr", "Tritanium");
    assert(panel.SetRouteCongestion(static_cast<uint32_t>(id), 0.7f));
    float c = panel.GetRouteCongestion(static_cast<uint32_t>(id));
    assert(c > 0.69f && c < 0.71f);
    // Clamping test
    assert(panel.SetRouteCongestion(static_cast<uint32_t>(id), 1.5f));
    c = panel.GetRouteCongestion(static_cast<uint32_t>(id));
    assert(c > 0.99f && c < 1.01f);
}

void test_trade_route_toggle() {
    TradeRoutePanel panel;
    int id = panel.AddRoute("Route1", "Jita", "Amarr", "Tritanium");
    assert(panel.ToggleRoute(static_cast<uint32_t>(id), false));
    assert(panel.EnabledRouteCount() == 0);
    assert(panel.ToggleRoute(static_cast<uint32_t>(id), true));
    assert(panel.EnabledRouteCount() == 1);
    assert(!panel.ToggleRoute(static_cast<uint32_t>(id), true)); // already enabled
}

void test_trade_route_total_stats() {
    TradeRoutePanel panel;
    int id1 = panel.AddRoute("R1", "Jita", "Amarr", "Tritanium");
    int id2 = panel.AddRoute("R2", "Amarr", "Dodixie", "Pyerite");
    panel.SetRouteVolume(static_cast<uint32_t>(id1), 1000.0f);
    panel.SetRouteVolume(static_cast<uint32_t>(id2), 2000.0f);
    panel.SetRouteRevenue(static_cast<uint32_t>(id1), 5000.0f);
    panel.SetRouteRevenue(static_cast<uint32_t>(id2), 8000.0f);
    float tv = panel.TotalVolume();
    assert(tv > 2999.0f && tv < 3001.0f);
    float tr = panel.TotalRevenue();
    assert(tr > 12999.0f && tr < 13001.0f);
}

void test_trade_route_disabled_excluded() {
    TradeRoutePanel panel;
    int id1 = panel.AddRoute("R1", "Jita", "Amarr", "Tritanium");
    int id2 = panel.AddRoute("R2", "Amarr", "Dodixie", "Pyerite");
    panel.SetRouteVolume(static_cast<uint32_t>(id1), 1000.0f);
    panel.SetRouteVolume(static_cast<uint32_t>(id2), 2000.0f);
    panel.ToggleRoute(static_cast<uint32_t>(id1), false);
    float tv = panel.TotalVolume();
    assert(tv > 1999.0f && tv < 2001.0f); // only R2 counts
}

void test_trade_route_max_routes() {
    TradeRoutePanel panel;
    for (int i = 0; i < 30; ++i) {
        int id = panel.AddRoute("R" + std::to_string(i), "A", "B", "C");
        assert(id > 0);
    }
    assert(panel.AddRoute("Extra", "A", "B", "C") == -1); // max reached
    assert(panel.RouteCount() == 30);
}

void test_trade_route_export_json() {
    TradeRoutePanel panel;
    panel.AddRoute("TestRoute", "Jita", "Amarr", "Tritanium");
    std::string json = panel.ExportJSON();
    assert(json.find("TestRoute") != std::string::npos);
    assert(json.find("Jita") != std::string::npos);
    assert(json.find("Amarr") != std::string::npos);
    assert(json.find("Tritanium") != std::string::npos);
}

void test_trade_route_headless_draw() {
    TradeRoutePanel panel;
    panel.AddRoute("R1", "Jita", "Amarr", "Tritanium");
    panel.Draw(); // should not crash without context
}
